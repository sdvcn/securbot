<template>
  <!-- Event page -->
  <div
    class="h-100 w-100"
  >
    <transition name="image-fade">
      <div
        v-if="eventImageURL"
        id="event-img-container"
        class="position-absolute w-100 h-100"
        style="z-index: 1000; top: 0;"
        @click="resetView"
      >
        <div
          class="position-absolute"
          style="top: 50%; left: 50%; transform: translate(-50%, -50%);"
        >
          <b-img
            thumbnail
            :src="eventImageURL"
            alt="There was a problem loading the image..."
            @mouseover="() => { imageHovered = true }"
            @mouseout="() => { imageHovered = false }"
          />
          <b-button-close
            class="position-absolute"
            style="top: 0.5rem; right: 1rem; opacity: 1; font-size: 2.5rem"
            text-variant="danger"
            @click="() => { $store.commit('database/clearEventImageURL') }"
          />
        </div>
      </div>
    </transition>
    <b-jumbotron
      id="event-layout"
      :fluid="true"
      :container-fluid="true"
      class="h-100 w-100 bg-transparent"
    >
      <b-row class="h-100">
        <!--                    LEFT COLUMN                    -->
        <b-col
          id="filter-col"
          md="3"
          class="mh-100"
        >
          <div
            id="filter-container"
            class="h-100 w-100 border rounded shadow-sb"
            style="max-height: 100%"
          >
            <!--                    TITLE                    -->
            <div class="d-flex flex-row justify-content-between">
              <h4 class="m-3">
                Filters
              </h4>
              <b-button
                class="m-3"
                size="sm"
                style="max-height: 60px"
                variant="outline-secondary"
                @click="applyFilter"
              >
                <font-awesome-icon
                  icon="redo"
                />
              </b-button>
            </div>
            <!--                    FILTERS                    -->
            <div
              id="inner-filter-container"
              class="border rounded mx-1 my-0 p-2 overflow-auto"
              style="height: calc( 100% - 60px - 50px);"
            >
              <b-container fluid>
                <!--                    SELECT FORMAT                    -->
                <b-row>
                  <b-form-select
                    v-model="selectedFilter"
                    :options="predefFilters"
                    text-field="name"
                    value-field="filters"
                    class="m-2"
                    @change="(event) => { $store.commit('database/setEventFilter', event) }"
                  >
                    <template v-slot:first>
                      <option
                        value=""
                        disabled
                      >
                        -- Predefined filters... --
                      </option>
                    </template>
                  </b-form-select>
                </b-row>
                <!--                    SELECT ROBOT                    -->
                <b-row>
                  <b-col
                    sm="4"
                  >
                    <label
                      for="robot-select-filter"
                      class="mt-2"
                    >
                      Robot:
                    </label>
                  </b-col>
                  <b-col
                    sm="8"
                  >
                    <b-form-select
                      id="robot-select-filter"
                      :value="selectedRobot"
                      class="ml-2"
                      :options="display"
                      :select-size="0"
                      @change="(event) => { $store.commit('database/setRobotFilter', event) }"
                    />
                  </b-col>
                </b-row>
                <!--                    AFTER DATE                    -->
                <b-row>
                  <b-col
                    sm="4"
                  >
                    <label
                      for="after-input-filter"
                      class="mt-2"
                    >
                      After:
                    </label>
                  </b-col>
                  <b-col
                    sm="8"
                  >
                    <b-form-input
                      id="after-input-filter"
                      :value="( eventFilter.after ? eventFilter.after.slice(0, 10) : '' )"
                      class="ml-2"
                      type="date"
                      @change="(event) =>
                        $store.commit('database/setEventFilter', { after: event })"
                    />
                  </b-col>
                </b-row>
                <!--                    BEFORE DATE                    -->
                <b-row>
                  <b-col
                    sm="4"
                  >
                    <label
                      for="before-input-filter"
                      class="mt-2"
                    >
                      Before:
                    </label>
                  </b-col>
                  <b-col
                    sm="8"
                  >
                    <b-form-input
                      id="before-input-filter"
                      :value="( eventFilter.before ? eventFilter.before.slice(0, 10) : '' )"
                      class="ml-2"
                      type="date"
                      @change="(event) =>
                        $store.commit('database/setEventFilter', { before: event })"
                    />
                  </b-col>
                </b-row>
                <!--                    OTHER FILTERS                    -->
                <b-row class="mt-2">
                  <h6
                    class="w-100"
                    style="margin-left: 15px;"
                  >
                    Other filters:
                  </h6>
                  <b-col>
                    <b-form-checkbox
                      id="new-filter-checkbox"
                      :checked="!!eventFilter.viewed"
                      @change="(event) =>
                        $store.commit('database/setEventFilter', { viewed: event })"
                    >
                      Only Viewed
                    </b-form-checkbox>
                  </b-col>
                  <b-col>
                    <b-form-checkbox
                      id="alert-filter-checkbox"
                      :checked="!!eventFilter.alert"
                      @change="(event) =>
                        $store.commit('database/setEventFilter', { alert: event })"
                    >
                      Alerts
                    </b-form-checkbox>
                  </b-col>
                </b-row>
                <!--                    TAG INLCUDE                    -->
                <b-row class="mt-3 position-relative">
                  <b-badge
                    class="position-absolute"
                    style="z-index: 100; top: -5px; left: 10px;"
                    variant="primary"
                  >
                    Include
                  </b-badge>
                  <b-col sm="12">
                    <div
                      id="include-tag-filter"
                      class="border rounded p-3 d-flex flex-row flex-wrap"
                      style="min-height: 80px;"
                    >
                      <button
                        v-for="tag in tagList"
                        :key="tag"
                        class="m-1"
                        :class="[ isIncludeTagSelected(tag)
                          ? 'btn btn-primary'
                          : 'btn btn-secondary' ]"
                        :disabled="isExcludeTagSelected(tag)"
                        @click="tagIncludeClicked(tag)"
                      >
                        {{ tag }}
                      </button>
                    </div>
                  </b-col>
                </b-row>
                <!--                    TAG EXCLUDE                    -->
                <b-row class="mt-3 position-relative">
                  <b-badge
                    class="position-absolute"
                    style="z-index: 100; top: -5px; left: 10px;"
                    variant="danger"
                  >
                    Exclude
                  </b-badge>
                  <b-col sm="12">
                    <div
                      id="exclude-tag-filter p-3"
                      class="border rounded p-3"
                      style="min-height: 80px;"
                    >
                      <button
                        v-for="tag in tagList"
                        :key="tag"
                        class="m-1"
                        :class="[ isExcludeTagSelected(tag)
                          ? 'btn btn-danger'
                          : 'btn btn-secondary' ]"
                        :disabled="isIncludeTagSelected(tag)"
                        @click="tagExcludeClicked(tag)"
                      >
                        {{ tag }}
                      </button>
                    </div>
                  </b-col>
                </b-row>
                <!--                    TEXT SEARCH                    -->
                <b-row class="my-3 position-relative">
                  <b-col>
                    <div id="search-for-filter">
                      <b-form-textarea
                        id="search-for-input-filter"
                        :value="(
                          eventFilter.search_expression
                            ? eventFilter.search_expression
                            : ''
                        )"
                        placeholder="Search for..."
                        rows="3"
                        max-rows="6"
                        @change="(event) =>
                          $store.commit('database/setEventFilter', { search_expression: event })"
                      />
                    </div>
                  </b-col>
                </b-row>
              </b-container>
            </div>
            <!--                    BUTTON                    -->
            <div
              style="height: 50px;"
            >
              <div class="h-100 w-100 d-flex flex-row-reverse align-items-center">
                <b-button
                  variant="primary"
                  class="mr-2"
                  style="max-height: 35px"
                  :disabled="querying"
                  @click="applyFilter"
                >
                  Apply Filters
                </b-button>
              </div>
            </div>
          </div>
        </b-col>
        <!--                    EVENT TABLE AND MAP                    -->
        <b-col
          id="table-col"
          md="9"
          class="mh-100"
        >
          <div
            id="table-container"
            class="h-100 w-100 position-relative"
          >
            <!--                    MAP                    -->
            <div
              v-if="isConnected && viewMap"
              class="position-absolute overlay-container-event"
            >
              <div
                id="event-overlay-button-container"
                class="overlay-button-container"
              >
                <!-- Zoom Map -->
                <b-button
                  id="increase-zoom-button"
                  squared
                  class="overlay-button"
                  @click="increaseZoom"
                >
                  <font-awesome-icon icon="plus" />
                </b-button>
                <b-tooltip
                  target="increase-zoom-button"
                  placement="left"
                  variant="secondary"
                >
                  Increase Map Zoom
                </b-tooltip>
                <!-- Unzoom Map -->
                <b-button
                  id="decrease-zoom-button"
                  squared
                  class="overlay-button"
                  @click="decreaseZoom"
                >
                  <font-awesome-icon icon="minus" />
                </b-button>
                <b-tooltip
                  target="decrease-zoom-button"
                  placement="left"
                  variant="secondary"
                >
                  Decrease Map Zoom
                </b-tooltip>
              </div>
            </div>
            <!--                    TOGGLE                    -->
            <div
              class="position-absolute"
              style="top:-35px;right:10px;z-index:10;"
            >
              <toggle-button
                :value="viewMap"
                :color="switchColor"
                :sync="true"
                :labels="{checked: 'map', unchecked: 'list'}"
                :disabled="!isConnected"
                @change="changeMapView"
              />
            </div>
            <!--                    TABLE                    -->
            <div
              class="w-100 h-100 border rounded shadow-sb"
            >
              <b-table
                v-if="!viewMap"
                id="event-bootstap-table"
                no-border-collapse
                hover
                striped
                show-empty
                empty-text="There is no events to show..."
                sticky-header="100%"
                :table-class="['m-0', 'table-rounded']"
                thead-class="text-center"
                tbody-class="text-sm-center"
                primary-key="_id"
                :fields="headers"
                :items="eventList"
                :busy="querying"
                :tbody-transition-props="transProps"
              >
                <template v-slot:table-busy>
                  <div class="text-center text-success my-2">
                    <b-spinner class="align-middle" />
                    <strong>Querying database...</strong>
                  </div>
                </template>
                <template v-slot:cell(time)="data">
                  {{ new Date(data.item.time).toLocaleString() }}
                </template>
                <template v-slot:cell(robot)="data">
                  {{ (robotIdToName(data.item.robot)
                    ? robotIdToName(data.item.robot)
                    : 'Unknown' ) }}
                </template>
                <template v-slot:cell(context)="data">
                  {{ (data.item.context
                    ? (Number(data.item.context)
                      ? `${(Number(data.item.context) * 100).toFixed(0)}%`
                      : data.item.context)
                    : 'No Context') }}
                </template>
                <template v-slot:cell(description_text)="data">
                  <div v-if="data.item.description_text">
                    <b-button
                      variant="info"
                      class="m-0 p-1"
                      style="font-size: 0.7rem; font-weight: bold;"
                      @click="data.toggleDetails"
                    >
                      {{ data.detailsShowing ? 'Hide' : 'Show' }} Description
                    </b-button>
                  </div>
                  <div v-else>
                    <b-button
                      variant="secondary"
                      class="m-0 p-1"
                      style="font-size: 0.7rem; font-weight: bold;"
                      disabled
                    >
                      No Desciption
                    </b-button>
                  </div>
                </template>
                <template v-slot:row-details="row">
                  <b-card>
                    {{ row.item.description_text }}
                  </b-card>
                </template>
                <template v-slot:cell(tags)="data">
                  <div
                    v-if="data.item.tags && data.item.tags.length"
                  >
                    <b-badge
                      v-for="tag in data.item.tags"
                      :key="tag"
                      size="sm"
                      variant="primary"
                      class="m-1"
                    >
                      {{ tag }}
                    </b-badge>
                  </div>
                  <div
                    v-else
                  >
                    <b-badge
                      variant="secondary"
                      class="m-1"
                      size="sm"
                    >
                      No Tag
                    </b-badge>
                  </div>
                </template>
                <template v-slot:cell(image)="data">
                  <div
                    v-if="data.item.files && data.item.files.length"
                  >
                    <b-button
                      :id="'image-btn-' + data.item.files[0].id"
                      class="bg-transparent m-0 p-0 border-0"
                      @click="() =>
                        $store.dispatch('database/setEventImageURL', data.item.files[0].id)"
                    >
                      <font-awesome-icon
                        icon="file-image"
                        :style="{ color: '#00A759' }"
                      />
                    </b-button>
                    <b-tooltip
                      :target="'image-btn-' + data.item.files[0].id"
                      placement="left"
                      variant="success"
                    >
                      Click to see the image
                    </b-tooltip>
                  </div>
                  <div v-else>
                    <b-button
                      :id="'image-btn-' + data.index"
                      class="bg-transparent m-0 p-0 border-0"
                    >
                      <font-awesome-icon
                        icon="exclamation-circle"
                        :style="{ color: 'red' }"
                      />
                    </b-button>
                    <b-tooltip
                      :target="'image-btn-' + data.index"
                      placement="left"
                      variant="danger"
                    >
                      There is not image available for this event...
                    </b-tooltip>
                  </div>
                </template>
              </b-table>
              <!--                    MAP VIDEO AND OVERLAY                    -->
              <div
                v-else-if="viewMap && isConnected"
                class="h-100 w-100 m-auto position-relative"
              >
                <video-box
                  :show="true"
                  :zoom="mapZoom"
                  :video-id="eventId"
                />
                <waypoint-overlay
                  :is-active="true"
                  :is-clickable="false"
                  :list="eventsWaypoints"
                  :zoom="mapZoom"
                  :map-size="mapSize"
                  :nb-of-waypoint="eventsWaypoints.length"
                  :video-element="eventElement"
                  :refresh-rate="1"
                />
              </div>
            </div>
          </div>
        </b-col>
      </b-row>
    </b-jumbotron>
  </div>
</template>

<script>
import { mapState, mapGetters } from 'vuex';
import { ToggleButton } from 'vue-js-toggle-button';
import VideoBox from '../widgets/VideoBox';
import WaypointOverlay from '../generic/WaypointOverlay';

/**
 * This page will be used to show events from a database. It will allow the operator to filter
 * those elements and should refresh when new data where sent to the database.
 *
 * @author Edouard Legare <edouard.legare@usherbrooke.ca>
 * @since 0.1.0
 * @displayName Events and Logging View
 * @version 0.1.0
 */
export default {
  name: 'event-page',
  components: {
    VideoBox,
    WaypointOverlay,
    ToggleButton,
  },
  data() {
    return {
      selectedFilter: '',
      viewMap: false,
      switchColor: {
        checked: '#00A759',
        unchecked: '#00A759',
        disabled: '#E8E8E8',
      },
      imageHovered: false,
      transProps: {
        name: 'flip-list',
      },
    };
  },
  computed: {
    ...mapGetters([
      'eventsWaypoints',
    ]),
    ...mapGetters('database', [
      'uri',
    ]),
    ...mapState({
      currentRobot: state => state.currentRobot,
      mapZoom: state => state.mapZoom,
      mapSize: state => state.mapSize,
      eventId: state => state.htmlElement.eventId,
      eventElement: state => state.htmlElement.event,
      isConnected: state => state.client.connectionState.robot === 'connected',
      headers: state => state.headers.events,
    }),
    ...mapState('database', {
      predefFilters: state => JSON.parse(JSON.stringify(state.predefFiltersFormat)),
      eventFilter: state => state.eventFilter,
      eventList: state => state.events,
      eventImageURL: state => state.eventImageURL,
      robots: state => state.robots,
      tagList: state => state.tagList,
      querying: state => state.queryingDB,
      queryError: state => state.errorDuringQuery,
      display: (state) => {
        // eslint-disable-next-line no-underscore-dangle
        const _display = [{ text: 'all', value: 'all' }];
        state.robots.forEach((robot) => {
          _display.push({
            text: robot.name,
            value: robot.id,
          });
        });
        return _display;
      },
      selectedRobot: (state) => {
        if (state.robotFilter.length === state.robots.length) {
          return 'all';
        }
        return (state.robotFilter.length ? state.robotFilter[0].id : '');
      },
    }),
  },
  mounted() {
    this.$store.dispatch('updateHTMLVideoElements');
  },
  methods: {
    resetView() {
      if (!this.imageHovered) {
        this.$store.commit('database/clearEventImageURL');
      }
    },
    robotIdToName(robotId) {
      for (const robot of this.display) {
        if (robot.value === robotId) {
          return robot.text;
        }
      }
      return '';
    },
    increaseZoom() {
      this.$store.commit('increaseMapZoom');
    },
    decreaseZoom() {
      this.$store.commit('decreaseMapZoom');
    },
    setLocalFilters(event) {
      const f = {
        includeTags: (event.tag_and ? event.tag_and : []),
        excludeTags: (event.tag_not ? event.tag_not : []),
        textSearch: (event.search_expression ? event.search_expression : ''),
        other: {
          onlyNew: event.viewed ? true : '',
          notify: event.alert ? true : '',
        },
        beforeDate: (event.before ? event.before.slice(0, 10) : ''),
        afterDate: (event.after ? event.after.slice(0, 10) : ''),
      };
      Object.assign(this.filters, f);
    },
    changeMapView(event) {
      this.viewMap = event.value;
      this.$nextTick(() => {
        this.$store.dispatch('updateHTMLVideoElements');
      });
    },
    isIncludeTagSelected(tag) {
      if (this.eventFilter.tag_and) return this.eventFilter.tag_and.includes(tag);
      return false;
    },
    isExcludeTagSelected(tag) {
      if (this.eventFilter.tag_not) return this.eventFilter.tag_not.includes(tag);
      return false;
    },
    tagIncludeClicked(tag) {
      if (this.eventFilter.tag_and) {
        const copy = JSON.parse(JSON.stringify(this.eventFilter.tag_and));
        if (copy.includes(tag)) {
          copy.splice(copy.indexOf(tag), 1);
        } else {
          copy.push(tag);
        }
        this.$store.commit('database/setEventFilter', { tag_and: copy });
      } else {
        this.$store.commit('database/setEventFilter', { tag_and: [tag] });
      }
    },
    tagExcludeClicked(tag) {
      if (this.eventFilter.tag_not) {
        const copy = JSON.parse(JSON.stringify(this.eventFilter.tag_not));
        if (copy.includes(tag)) {
          copy.splice(copy.indexOf(tag), 1);
        } else {
          copy.push(tag);
        }
        this.$store.commit('database/setEventFilter', { tag_not: copy });
      } else {
        this.$store.commit('database/setEventFilter', { tag_not: [tag] });
      }
    },
    applyFilter() {
      this.viewMap = false;
      this.$store.commit('database/resetQuery');
      this.$store.commit('database/resetEvents');
      this.$store.dispatch('database/filterEvents');
    },
  },
};
</script>

<style>
#event-img-container {
  backdrop-filter: blur(10px);
}
.image-fade-enter-active, .image-fade-leave-active {
  transition: opacity .5s;
}
.image-fade-enter, .image-fade-leave-to {
  opacity: 0;
}
.flip-list-move {
  transition: transform 0.5s;
}
.flip-list-enter-active {
  transition: all 1s;
}
.flip-list-leave-active {
  transition: all 0.1s;
}
.flip-list-enter, .flip-list-leave-to {
  opacity: 0;
}
.table-b-table-default {
  background-color: #00A759 !important;
  color: white !important;
}
.table-rounded {
  border: solid 1px #00A759;
  border-radius: 0.25rem;
  border-collapse: separate;
}
.table-rounded th {
  border-color: white;
  border-top: none;
}
.table-rounded tr:last-child > td {
  border-bottom: none;
}
.overlay-button {
  background-color: #b5b5b5;
  opacity: 0.4;
  height: 60px !important;
  width: 60px !important;
}
.overlay-button:disabled {
  opacity: 0.2;
  background-color: grey;
}
.overlay-button-container {
  padding: 7px;
  background-color: rgba(245, 245, 245, 0.75);
  /* border: solid;
  border-color: black; */
  border-radius: 5px 0 0 5px;
  margin: auto;
  margin-bottom: 5px;
}
.overlay-container-event {
  top: 5px;
  right: 0px;
  z-index: 100;
  max-width: 80px;
  height: auto;
}
</style>
