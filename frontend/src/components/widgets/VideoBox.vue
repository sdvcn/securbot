<template>
  <!-- Video Box widget -->
  <div
    id="VideoStream"
    class="w-100 h-100 m-auto position-relative bg-black-sb shadow-sb"
  >
    <!-- Video container -->
    <div class="w-100 h-100 position-relative overflow-hidden">
      <!-- Video -->
      <video
        v-show="show"
        :id="videoId"
        :style="videoZoomTransform"
        class="w-100 h-100"
      />
    </div>
  </div>
</template>

<script>
/**
 * A component that sets the html of a video element to be used the api.
 *
 * Authors:
 *
 *    - Edouard Legare - <edouard.legare@usherbrooke.ca>
 * @since 0.1.0
 * @version 1.0.0
 * @displayName Video Box
 */

export default {
  name: 'video-box',
  props: {
    /**
     * The html id of the video.
     */
    videoId: {
      type: String,
      required: true,
    },
    /**
     * The status indicating if the video should be shown or not.
     */
    show: {
      type: Boolean,
      required: true,
    },
    zoom: {
      type: Number,
      default: 1,
    },
  },
  computed: {
    videoZoomTransform() {
      return {
        '-moz-transform': `scale(${this.zoom})`,
        '-webkit-transform': `scale(${this.zoom})`,
        '-o-transform': `scale(${this.zoom})`,
        '-ms-transform': `scale(${this.zoom})`,
        transform: `scale(${this.zoom})`,
      };
    },
  },
};
</script>

<style>
</style>
