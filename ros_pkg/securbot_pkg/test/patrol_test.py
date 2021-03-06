#!/usr/bin/env python
PKG = 'securbot_pkg'

import sys, json, unittest, rospy, time, actionlib
from collections import deque
from std_msgs.msg import String
from geometry_msgs.msg import PoseStamped
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal

FAKE_PATROL_1 = '{ "patrol":[ {"x":1,"y":2,"yaw":3}, {"x":2,"y":1,"yaw":4}, {"x":3,"y":0,"yaw":3}, {"x":4,"y":-1,"yaw":4} ], "loop": false }'
INTERRUPT_TRUE_JSON = json.dumps({"interrupt":True})
TIMEOUT = 2 # seconds

class  PatrolTestSuite(unittest.TestCase):
    def __init__(self, *args):
        super(PatrolTestSuite, self).__init__(*args)
        rospy.init_node('patrol_test')
        # Move Base Mock
        self.actionServer = actionlib.SimpleActionServer(
                'move_base',
                MoveBaseAction,
                auto_start = False)
        self.actionServer.start()
        # Electron interface
        self.patrolPublisher = rospy.Publisher('/electron/patrol', String, queue_size = 5)
        self.patrolCanceller = rospy.Publisher('/electron/patrol_halt', String, queue_size = 5)
        rospy.Subscriber('/electron/patrol_feedback', String, self.waypointDoneCallback)
        # Map Image Mock
        self.conversionRequests = []

        # Waypoints Status Mock
        self.waypointsDoneStatus = deque()

        self.mapImageOutput = rospy.Publisher('/map_image_generator/output_goal',
                PoseStamped, queue_size = 20)
        rospy.Subscriber('/map_image_generator/input_goal', PoseStamped,
                self.mapImageCallBack)
        rospy.sleep(0.5) # waiting for subscribers and publishers to come online

    def mapImageCallBack(self, data):
        self.conversionRequests.append(data)

    def waypointDoneCallback(self, message):
        self.waypointsDoneStatus.append(json.loads(message.data))

    # Setting up the test environment before every test
    def setUp(self):
        self.conversionRequests = []
        self.waypointsDoneStatus.clear()


    #
    # TEST FUNCTIONS
    #

    ## Regular waypoint navigation
    def test_regular_patrol(self):
        self.patrolPublisher.publish(FAKE_PATROL_1)
        patrolDict = json.loads(FAKE_PATROL_1)

        deadline = time.time() + TIMEOUT
        while not rospy.is_shutdown() and len(self.conversionRequests) < 4 and time.time() < deadline:
            rospy.sleep(0.1)

        self.assertEquals(len(self.conversionRequests), 4,
                'Move Base received :' + str(len(self.conversionRequests)) + ' elements')

        ## Send converted waypoints
        for waypoint in self.conversionRequests:
            # Modifying waypoint to access correct waypoints are registered
            waypoint.pose.position.x += 1
            self.mapImageOutput.publish(waypoint)
        del self.conversionRequests[:]

        # Checking for round start message
        deadline = time.time() + TIMEOUT
        while not rospy.is_shutdown() and len(self.waypointsDoneStatus) < 1 and time.time() < deadline:
            rospy.sleep(0.1)
        self.assertGreater(len(self.waypointsDoneStatus), 0)
        startMessage = self.waypointsDoneStatus.popleft()
        self.assertEquals(startMessage['status'], 'start')

        ## Process each waypoint as if it had been navigated to
        for index, waypoint in enumerate(patrolDict['patrol']):
            deadline = time.time() + TIMEOUT
            while not rospy.is_shutdown() and not self.actionServer.is_new_goal_available() and time.time() < deadline:
                rospy.sleep(0.1)

            # Assert messages being sent to movebase
            self.assertTrue(self.actionServer.is_new_goal_available(),
                    'Expecting move_base to receive a goal')

            # Make sure the waypoint received matches
            goal = self.actionServer.accept_new_goal()
            self.actionServer.set_succeeded()
            self.assertIsInstance(goal, MoveBaseGoal)
            self.assertEquals(goal.target_pose.pose.position.x, waypoint['x'] + 1)

            # Make sure waypoint completion is reported
            deadline = time.time() + TIMEOUT
            while not rospy.is_shutdown() and len(self.waypointsDoneStatus) < 1 and time.time() < deadline:
                rospy.sleep(0.1)
            self.assertGreater(len(self.waypointsDoneStatus), 0)

            doneStatus = self.waypointsDoneStatus.popleft()
            self.assertEquals(doneStatus['goalsReached'], index + 1, 'Waypoint completion feedback failed')


    ## Testing waypoint cancellation after waypoint conversion by map image
    def test_waypoint_is_cancelled(self):

        ## Publish patrol plan
        self.patrolPublisher.publish(FAKE_PATROL_1)

        # Convert waypoints
        deadline = time.time() + TIMEOUT
        while not rospy.is_shutdown() and len(self.conversionRequests) < 4 and time.time() < deadline:
            rospy.sleep(0.1)
        self.assertEquals(len(self.conversionRequests), 4,
                'Move Base received :' + str(len(self.conversionRequests)) + ' elements')
        for waypoint in self.conversionRequests:
            # Modifying waypoint to access correct waypoints are registered
            waypoint.pose.position.x += 1
            self.mapImageOutput.publish(waypoint)
        del self.conversionRequests[:]

        # Checking for round start message
        deadline = time.time() + TIMEOUT
        while not rospy.is_shutdown() and len(self.waypointsDoneStatus) < 1 and time.time() < deadline:
            rospy.sleep(0.1)
        self.assertGreater(len(self.waypointsDoneStatus), 0)
        startMessage = self.waypointsDoneStatus.popleft()
        self.assertEquals(startMessage['status'], 'start')

        # Confirm the first move_base goal is received
        deadline = time.time() + TIMEOUT
        while not rospy.is_shutdown() and not self.actionServer.is_new_goal_available() and time.time() < deadline:
            rospy.sleep(0.1)
        self.assertTrue(self.actionServer.is_new_goal_available(),
                'Expecting move_base to receive a goal')
        # Accept first move_base goal
        self.actionServer.accept_new_goal()

        ## Publish interrupt
        self.patrolCanceller.publish(INTERRUPT_TRUE_JSON)

        ## Assert waypoint have been interrupted
        deadline = time.time() + TIMEOUT
        while not rospy.is_shutdown() and len(self.waypointsDoneStatus) < 1 and time.time() < deadline:
            rospy.sleep(0.1)
        self.assertGreater(len(self.waypointsDoneStatus), 0)
        startMessage = self.waypointsDoneStatus.popleft()
        self.assertEquals(startMessage['status'], 'interrupt')

        ## Assert waypoint cancellation request
        deadline = time.time() + TIMEOUT
        while not rospy.is_shutdown() and not self.actionServer.is_preempt_requested() and time.time() < deadline:
            rospy.sleep(0.1)
        self.assertTrue(self.actionServer.is_preempt_requested(),
                'Expecting move_base action cancellation')


if __name__ == '__main__':
    import rostest
    rostest.run(PKG, 'patrol_exec', PatrolTestSuite)
