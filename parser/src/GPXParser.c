/*
 * CIS2750 A3
 * Name: Hyrum Nantais
 * Student Number: 1105303
 */

#include "GPXParser.h"
#include "GPXHelpers.h"

GPXdoc* createGPXdoc(char* fileName)
{
    if (fileName == NULL)
    {
        return NULL;
    }

    LIBXML_TEST_VERSION

    /* parse file - libxml function */
    xmlDoc *doc = xmlReadFile(fileName, NULL, 0);

    if (doc == NULL) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* get root node - libxml function */
    xmlNode *root = xmlDocGetRootElement(doc);
    GPXdoc *GPXdocument = initializeGPXdoc();
    if (!generateGPXdoc(GPXdocument, root))
    {
        deleteGPXdoc(GPXdocument);
        GPXdocument = NULL;
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return GPXdocument;
}

char* GPXdocToString(GPXdoc* doc)
{
    char *retValue = calloc(1, sizeof(char));
    if (doc != NULL)
    {
        retValue = realloc(retValue, 60 + strlen(doc->namespace) + strlen(doc->creator));
        memset(retValue, 0, 60 + strlen(doc->namespace) + strlen(doc->creator));
        sprintf(retValue, "GPX Document namespace: %s, version: %.1f, creator: %s", doc->namespace, doc->version, doc->creator);
        
        /* Reallocate strings? */
        char *wptString = toString(doc->waypoints);
        retValue = realloc(retValue, strlen(retValue) + strlen(wptString) + 1);
        strcat(retValue, wptString);
        free(wptString);

        char *rteString = toString(doc->routes);
        retValue = realloc(retValue, strlen(retValue) + strlen(rteString) + 1);
        strcat(retValue, rteString);
        free(rteString);

        char *trkString = toString(doc->tracks);
        retValue = realloc(retValue, strlen(retValue) + strlen(trkString) + 2);
        strcat(retValue, trkString);
        free(trkString);

        strcat(retValue, "\n");
    }
    return retValue;
}

void deleteGPXdoc(GPXdoc* doc)
{
    if (doc != NULL)
    {
        freeList(doc->routes);
        freeList(doc->tracks);
        freeList(doc->waypoints);
        free(doc->creator);
        free(doc);
    }
}

int getNumWaypoints(const GPXdoc* doc)
{
    if (doc != NULL)
    {
        return getLength(doc->waypoints);
    }
    return 0;
}

int getNumRoutes(const GPXdoc* doc)
{
    if (doc != NULL)
    {
        return getLength(doc->routes);
    }
    return 0;
}

int getNumTracks(const GPXdoc* doc)
{
    if (doc != NULL)
    {
        return getLength(doc->tracks);
    }
    return 0;
}

int getNumSegments(const GPXdoc* doc)
{
    if (doc != NULL)
    {
        int count = 0;
        ListIterator trks = createIterator(doc->tracks);
        void *ele;
        while ((ele = nextElement(&trks)) != NULL)
        {
            Track *trk = (Track *)ele;
            count += getLength(trk->segments);
        }
        return count;
    }
    return 0;
}

int getNumGPXData(const GPXdoc* doc)
{
    if (doc != NULL)
    {
        int count = 0;
        /* Go through waypoints of doc */
        count += getGPXDataInWaypoint(createIterator(doc->waypoints));

        /* Go through routes of doc */
        ListIterator rtes = createIterator(doc->routes);
        void *ele;
        while ((ele = nextElement(&rtes)) != NULL)
        {
            Route *rte = (Route *)ele;
            count += getLength(rte->otherData);
            if (strcmp(rte->name, "") != 0) {
                count++;
            }
            count += getGPXDataInWaypoint(createIterator(rte->waypoints));
        }

        /* Go through tracks of doc */
        ListIterator trks = createIterator(doc->tracks);
        while ((ele = nextElement(&trks)) != NULL)
        {
            Track *trk = (Track *)ele;
            count += getLength(trk->otherData);
            if (strcmp(trk->name, "") != 0) {
                count++;
            }
            /* Go through track segments of track */
            ListIterator trksegs = createIterator(trk->segments);
            void *eleTwo;
            while ((eleTwo = nextElement(&trksegs)) != NULL)
            {
                TrackSegment *trkseg = (TrackSegment *)eleTwo;
                count += getGPXDataInWaypoint(createIterator(trkseg->waypoints));
            }
        }
        return count;
    }
    return 0;
}

Waypoint* getWaypoint(const GPXdoc* doc, char* name)
{
    if (doc != NULL && name != NULL)
    {
        /* Search docs waypoints */
        Waypoint *wpt = findWaypointInList(createIterator(doc->waypoints), name);
        if (wpt != NULL)
        {
            return wpt;
        }

        /* Go through routes of doc */
        ListIterator rtes = createIterator(doc->routes);
        void *ele;
        while ((ele = nextElement(&rtes)) != NULL)
        {
            Route *rte = (Route *)ele;
            Waypoint *wpt = findWaypointInList(createIterator(rte->waypoints), name);
            if (wpt != NULL)
            {
                return wpt;
            }
        }

        /* Go through tracks of doc */
        ListIterator trks = createIterator(doc->tracks);
        while ((ele = nextElement(&trks)) != NULL)
        {
            Track *trk = (Track *)ele;
            
            /* Go through track segments of track */
            ListIterator trksegs = createIterator(trk->segments);
            void *eleTwo;
            while ((eleTwo = nextElement(&trksegs)) != NULL)
            {
                TrackSegment *trkseg = (TrackSegment *)eleTwo;
                Waypoint *wpt = findWaypointInList(createIterator(trkseg->waypoints), name);
                if (wpt != NULL)
                {
                    return wpt;
                }
            }
        }
    }
    return NULL;
}

Track* getTrack(const GPXdoc* doc, char* name)
{
    if (doc != NULL && name != NULL)
    {
        ListIterator tracks = createIterator(doc->tracks);
        void *ele;
        while ((ele = nextElement(&tracks)) != NULL)
        {
            Track *trk = (Track *)ele;
            if (strcmp(trk->name, name) == 0)
            {
                return trk;
            }
        }
    }
    return NULL;
}

Route* getRoute(const GPXdoc* doc, char* name)
{
    if (doc != NULL && name != NULL)
    {
        ListIterator routes = createIterator(doc->routes);
        void *ele;
        while ((ele = nextElement(&routes)) != NULL)
        {
            Route *rte = (Route *)ele;
            if (strcmp(rte->name, name) == 0)
            {
                return rte;
            }
        }
    }
    return NULL;
}

/* START OF HELPER FUNCTIONS */
void deleteGpxData(void* data)
{
    if (data != NULL)
    {
        free(data);
    }
}

char* gpxDataToString(void* data)
{
    char *retValue = calloc(1, sizeof(char));
    if (data != NULL)
    {
        GPXData *otherData = (GPXData *)data;
        retValue = realloc(retValue, 35 + strlen(otherData->value) + strlen(otherData->name));
        sprintf(retValue, "{\"name\":\"%s\",\"value\":\"%s\"}", otherData->name, otherData->value);
    }
    return retValue;
}

int compareGpxData(const void *first, const void *second)
{
    return first == second;
}

void deleteWaypoint(void* data)
{
    if (data != NULL)
    {
        Waypoint *wpt = (Waypoint *)data;
        freeList(wpt->otherData);
        free(wpt->name);
        free(wpt);
    }
}

char* waypointToString(void* data)
{
    char *retValue = calloc(1, sizeof(char));
    if (data != NULL)
    {
        Waypoint *wpt = (Waypoint *)data;
        retValue = realloc(retValue, 80 + strlen(wpt->name));
        if (wpt->name[0] == '\0')
        {
            sprintf(retValue, "\tWaypoint: no name, latitude: %.5f, longitude: %.5f", wpt->latitude, wpt->longitude);
        } else
        {
            sprintf(retValue, "\tWaypoint name: %s, latitude: %.5f, longitude: %.5f", wpt->name, wpt->latitude, wpt->longitude);
        }
        
        char *otherDataString = toString(wpt->otherData);
        retValue = realloc(retValue, strlen(retValue) + strlen(otherDataString) + 1);
        strcat(retValue, otherDataString);
        free(otherDataString);
    }
    return retValue;
}

int compareWaypoints(const void *first, const void *second)
{
    return first == second;
}

void deleteRoute(void* data)
{
    if (data != NULL)
    {
        Route *rte = (Route *)data;
        freeList(rte->waypoints);
        freeList(rte->otherData);
        free(rte->name);
        free(rte);
    }
}

char* routeToString(void* data)
{
    char *retValue = calloc(1, sizeof(char));
    if (data != NULL)
    {
        Route *rte = (Route *)data;
        retValue = realloc(retValue, 25 + strlen(rte->name));
        if (rte->name[0] == '\0')
        {
            sprintf(retValue, "\tRoute: no name");
        } else
        {
            sprintf(retValue, "\tRoute name: %s", rte->name);
        }

        char *otherDataString = toString(rte->otherData);
        retValue = realloc(retValue, strlen(retValue) + strlen(otherDataString) + 1);
        strcat(retValue, otherDataString);
        free(otherDataString);

        char *wptString = toString(rte->waypoints);
        retValue = realloc(retValue, strlen(retValue) + strlen(wptString) + 1);
        strcat(retValue, wptString);
        free(wptString);
    }
    return retValue;
}

int compareRoutes(const void *first, const void *second)
{
    return first == second;
}

void deleteTrackSegment(void* data)
{
    if (data != NULL)
    {
        TrackSegment *trkseg = (TrackSegment *)data;
        freeList(trkseg->waypoints);
        free(trkseg);
    }
}

char* trackSegmentToString(void* data)
{
    char *retValue = calloc(1, sizeof(char));
    if (data != NULL)
    {
        retValue = realloc(retValue, 25 * sizeof(char));
        TrackSegment *trkseg = (TrackSegment *)data;
        sprintf(retValue, "\tTrackSegment: ");
        
        char *wptString = toString(trkseg->waypoints);
        retValue = realloc(retValue, strlen(retValue) + strlen(wptString) + 1);
        strcat(retValue, wptString);
        free(wptString);
        
        return retValue;
    }
    return retValue;
}

int compareTrackSegments(const void *first, const void *second)
{
    return first == second;
}

void deleteTrack(void* data)
{
    if (data != NULL)
    {
        Track *trk = (Track *)data;
        freeList(trk->segments);
        freeList(trk->otherData);
        free(trk->name);
        free(trk);
    }
}

char* trackToString(void* data)
{
    char *retValue = calloc(1, sizeof(char));
    if (data != NULL)
    {
        Track *trk = (Track *)data;
        retValue = realloc(retValue, 25 + strlen(trk->name));
        if (trk->name[0] == '\0')
        {
            sprintf(retValue, "\tTrack: no name");
        } else
        {
            sprintf(retValue, "\tTrack name: %s", trk->name);
        }

        char *otherDataString = toString(trk->otherData);
        retValue = realloc(retValue, strlen(retValue) + strlen(otherDataString) + 1);
        strcat(retValue, otherDataString);
        free(otherDataString);

        char *segmentsString = toString(trk->segments);
        retValue = realloc(retValue, strlen(retValue) + strlen(segmentsString) + 1);
        strcat(retValue, segmentsString);
        free(segmentsString);
    }
    return retValue;
}

int compareTracks(const void *first, const void *second)
{
    return first == second;
}
/* END OF HELPER FUNCTIONS */


/* START OF A2 FUNCTIONS */
GPXdoc* createValidGPXdoc(char *fileName, char *gpxSchemaFile)
{
    if (fileName == NULL || gpxSchemaFile == NULL)
    {
        return NULL;
    }

    LIBXML_TEST_VERSION

    /* parse file - libxml function */
    xmlDoc *doc = xmlReadFile(fileName, NULL, 0);

    if (doc == NULL) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Validate xml document with schema before continuing */
    if (!validateXMLDoc(doc, gpxSchemaFile))
    {
        return NULL;
    }

    /* get root node - libxml function */
    xmlNode *root = xmlDocGetRootElement(doc);
    GPXdoc *GPXdocument = initializeGPXdoc();
    if (!generateGPXdoc(GPXdocument, root))
    {
        deleteGPXdoc(GPXdocument);
        GPXdocument = NULL;
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return GPXdocument;
}

bool writeGPXdoc(GPXdoc *doc, char *fileName)
{
    /* Check pointers and filename file extension */
    if (doc == NULL || fileName == NULL || strlen(fileName) < 5 || strcmp(fileName + strlen(fileName) - 4, ".gpx") != 0)
    {
        return false;
    }

    int garbage;
    /* Convert gpx to xml document and print document to file */
    xmlDoc *xml = xmlDocConvert(doc, &garbage);
    xmlSaveFormatFileEnc(fileName, xml, "UTF-8", 1);

    xmlFreeDoc(xml);
    xmlCleanupParser();
    return true;
}

bool validateGPXDoc(GPXdoc *gpxDoc, char *gpxSchemaFile)
{
    /* Check pointers and schema filename file extension */
    if (gpxDoc == NULL || gpxSchemaFile == NULL || strlen(gpxSchemaFile) < 5 || strcmp(gpxSchemaFile + strlen(gpxSchemaFile) - 4, ".xsd") != 0)
    {
        return false;
    }

    /* Validate pointers and data within gpxDoc, while converting gpx doc to xml doc */
    int dataValid = 1;
    xmlDoc *doc = xmlDocConvert(gpxDoc, &dataValid);
    if (dataValid == 0)
    {
        xmlFreeDoc(doc);
        return false;
    }

    dataValid = validateXMLDoc(doc, gpxSchemaFile);
    /* Free xml document if it was not freed due to being valid */
    if (dataValid)
    {
        xmlFreeDoc(doc);
    }
    return dataValid;
}

float round10(float len)
{
    float temp = fmodf(len, 10);
    if (temp < 5)
    {
        return len - temp;
    }
    return len + 10 - temp;
}

float getRouteLen(const Route *rt)
{
    if (rt == NULL || rt->waypoints == NULL)
    {
        return 0;
    }
    
    ListIterator wpts = createIterator(rt->waypoints);
    Waypoint *wpt = (Waypoint *)nextElement(&wpts);
    float dist = 0;
    if (wpt != NULL)
    {
        Waypoint *wptTwo;
        while ((wptTwo = (Waypoint *)nextElement(&wpts)) != NULL)
        {
            dist += calculateDistance(wpt, wptTwo);
            wpt = wptTwo;
        }
    }
    return dist;
}

float getTrackLen(const Track *tr)
{
    if (tr == NULL || tr->segments == NULL)
    {
        return 0;
    }

    float dist = 0;
    Waypoint *first = NULL;
    Waypoint *last = NULL;
    ListIterator segments = createIterator(tr->segments);
    
    /* Get first segment in track */
    TrackSegment *trkseg = (TrackSegment *)nextElement(&segments);
    if (trkseg != NULL && trkseg->waypoints != NULL)
    {
        /* Calculate distance of first segment */
        ListIterator wpts = createIterator(trkseg->waypoints);
        Waypoint *wpt = (Waypoint *)nextElement(&wpts);
        if (wpt != NULL)
        {
            Waypoint *wptTwo;
            while ((wptTwo = (Waypoint *)nextElement(&wpts)) != NULL)
            {
                dist += calculateDistance(wpt, wptTwo);
                wpt = wptTwo;
            }
            last = wpt;
        }

        /* Loop through segments after */
        while ((trkseg = (TrackSegment *)nextElement(&segments)) != NULL)
        {
            if (trkseg->waypoints != NULL)
            { 
                /* Calculate distance of next segment */
                wpts = createIterator(trkseg->waypoints);
                wpt = (Waypoint *)nextElement(&wpts);
                if (wpt != NULL)
                {
                    first = wpt;
                    Waypoint *wptTwo;
                    while ((wptTwo = (Waypoint *)nextElement(&wpts)) != NULL)
                    {
                        dist += calculateDistance(wpt, wptTwo);
                        wpt = wptTwo;
                    }
                }
                /* Calculate distance between two segments and setup last point for next run through */
                dist += calculateDistance(last, first);
                last = wpt;
            }
        }
    }
    return dist;
}

int numRoutesWithLength(const GPXdoc *doc, float len, float delta)
{
    if (doc == NULL || len < 0 || delta < 0 || doc->routes == NULL)
    {
        return 0;
    }
    float lowerTolerance = len - delta;
    float higherTolerance = len + delta;
    int num = 0;

    ListIterator rtes = createIterator(doc->routes);
    Route *rte;
    while ((rte = (Route *)nextElement(&rtes)) != NULL)
    {
        float dist = getRouteLen(rte);
        if (dist >= lowerTolerance && dist <= higherTolerance)
        {
            num++;
        }
    }
    return num;
}

int numTracksWithLength(const GPXdoc *doc, float len, float delta)
{
    if (doc == NULL || len < 0 || delta < 0 || doc->tracks == NULL)
    {
        return 0;
    }
    float lowerTolerance = len - delta;
    float higherTolerance = len + delta;
    int num = 0;

    ListIterator trks = createIterator(doc->tracks);
    Track *trk;
    while ((trk = (Track *)nextElement(&trks)) != NULL)
    {
        float dist = getTrackLen(trk);
        if (dist >= lowerTolerance && dist <= higherTolerance)
        {
            num++;
        }
    }
    return num;
}

bool isLoopRoute(const Route *rt, float delta)
{
    if (rt == NULL || delta < 0 || getLength(rt->waypoints) < 4)
    {
        return false;
    }

    /* Calculate first and last point distance */
    Waypoint *first = (Waypoint *)getFromFront(rt->waypoints);
    Waypoint *last = (Waypoint *)getFromBack(rt->waypoints);
    if (calculateDistance(first, last) <= delta)
    {
        return true;
    }
    return false;
}

bool isLoopTrack(const Track *tr, float delta)
{
    if (tr == NULL || delta < 0)
    {
        return false;
    }

    TrackSegment *trkseg;
    int numWaypoints = 0;
    /* Count number of waypoints in track */
    ListIterator segments = createIterator(tr->segments);
    while ((trkseg = (TrackSegment *)nextElement(&segments)) != NULL)
    {
        numWaypoints += getLength(trkseg->waypoints);
    }

    if (numWaypoints < 4)
    {
        return false;
    }

    /* Calculate first and last point distance */
    TrackSegment *firstSegment = (TrackSegment *)getFromFront(tr->segments);
    TrackSegment *lastSegment = (TrackSegment *)getFromBack(tr->segments);
    Waypoint *first = (Waypoint *)getFromFront(firstSegment->waypoints);
    Waypoint *last = (Waypoint *)getFromBack(lastSegment->waypoints);
    if (calculateDistance(first, last) <= delta)
    {
        return true;
    }
    return false;
}

List *getRoutesBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    if (doc != NULL && delta >= 0)
    {
        /* Create waypoints with source and dest coordinates */
        Waypoint source;
        Waypoint dest;
        source.latitude = sourceLat;
        source.longitude = sourceLong;
        dest.latitude = destLat;
        dest.longitude = destLong;

        List *retList = initializeList(&routeToString, &dummy, &compareRoutes);
        Route *rte;
        ListIterator routes = createIterator(doc->routes);
        while ((rte = (Route *)nextElement(&routes)) != NULL)
        {
            /* Make sure route has at least 2 waypoints in it */
            if (getLength(rte->waypoints) >= 2)
            {
                Waypoint *first = (Waypoint *)getFromFront(rte->waypoints);
                Waypoint *last = (Waypoint *)getFromBack(rte->waypoints);
                /* Add route to list of routes if its between source and dest */
                if (calculateDistance(first, &source) <= delta && calculateDistance(last, &dest) <= delta)
                {
                    insertBack(retList, rte);
                }
            }
        }
        /* Return list if there are routes in list */
        if (getLength(retList) > 0)
        {
            return retList;
        }
        freeList(retList);
    }
    return NULL;
}

List *getTracksBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    if (doc != NULL && delta >= 0)
    {
        /* Create waypoints with source and dest coordinates */
        Waypoint source;
        Waypoint dest;
        source.latitude = sourceLat;
        source.longitude = sourceLong;
        dest.latitude = destLat;
        dest.longitude = destLong;

        List *retList = initializeList(&trackToString, &dummy, &compareTracks);
        Track *trk;
        ListIterator tracks = createIterator(doc->tracks);
        while ((trk = (Track *)nextElement(&tracks)) != NULL)
        {
            TrackSegment *firstSegment = (TrackSegment *)getFromFront(trk->segments);
            TrackSegment *lastSegment = (TrackSegment *)getFromBack(trk->segments);
            /* Make sure segments have at least 1 waypoint in them */
            if (getLength(firstSegment->waypoints) >= 1 && getLength(lastSegment->waypoints) >= 1)
            {
                Waypoint *first = (Waypoint *)getFromFront(firstSegment->waypoints);
                Waypoint *last = (Waypoint *)getFromBack(lastSegment->waypoints);
                /* Add track to list of tracks if its between source and dest */
                if (calculateDistance(first, &source) <= delta && calculateDistance(last, &dest) <= delta)
                {
                    insertBack(retList, trk);
                }
            }
        }
        /* Return list if there are tracks in list */
        if (getLength(retList) > 0)
        {
            return retList;
        }
        freeList(retList);
    }
    return NULL;
}

char* routeToJSON(const Route *rte)
{
    char *retValue = calloc(3, sizeof(char));
    if (rte != NULL)
    {
        retValue = realloc(retValue, 150 + strlen(rte->name));
        char trueFalse[10];

        /* Get true or false string if the route is a loop */
        if (isLoopRoute(rte, 10))
        {
            sprintf(trueFalse, "true");
        } else
        {
            sprintf(trueFalse, "false");
        }

        if (strcmp(rte->name, "") == 0)
        {
            sprintf(retValue, "{\"name\":\"\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", getLength(rte->waypoints), round10(getRouteLen(rte)), trueFalse);
        } else
        {
            sprintf(retValue, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", rte->name, getLength(rte->waypoints), round10(getRouteLen(rte)), trueFalse);
        }
    } else
    {
        strcpy(retValue, "{}");
    }
    return retValue;
}

char* waypointToJSON(const Waypoint *wpt, int i)
{
    char *retValue = calloc(3, sizeof(char));
    if (wpt != NULL)
    {
        retValue = realloc(retValue, 60 + strlen(wpt->name));
        sprintf(retValue, "{\"name\":\"%s\",\"lat\":%f,\"lon\":%f,\"index\":%d}", wpt->name, wpt->latitude, wpt->longitude, i);
    } else
    {
        strcpy(retValue, "{}");
    }
    return retValue;
}

char* trackToJSON(const Track *trk)
{
    char *retValue = calloc(3, sizeof(char));
    if (trk != NULL)
    {
        retValue = realloc(retValue, 150 + strlen(trk->name));
        char trueFalse[10];

        int numWaypoints = 0;
        /* Count number of waypoints in track */
        ListIterator segments = createIterator(trk->segments);
        TrackSegment *trkseg;
        while ((trkseg = (TrackSegment *)nextElement(&segments)) != NULL)
        {
            numWaypoints += getLength(trkseg->waypoints);
        }


        /* Get true or false string if the route is a loop */
        if (isLoopTrack(trk, 10))
        {
            sprintf(trueFalse, "true");
        } else
        {
            sprintf(trueFalse, "false");
        }

        if (strcmp(trk->name, "") == 0)
        {
            sprintf(retValue, "{\"name\":\"\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", numWaypoints, round10(getTrackLen(trk)), trueFalse);
        } else
        {
            sprintf(retValue, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", trk->name, numWaypoints, round10(getTrackLen(trk)), trueFalse);
        }
    } else
    {
        strcpy(retValue, "{}");
    }
    return retValue;
}

char* routeListToJSON(const List *list)
{
    char *retValue = calloc(5, sizeof(char));
    strcpy(retValue, "[");

    List *ls = (List *)list;
    if (list != NULL && getLength(ls) > 0)
    {
        ListIterator routes = createIterator(ls);
        Route *rte;
        while ((rte = (Route *)nextElement(&routes)) != NULL)
        {
            char *rteString = routeToJSON(rte);
            retValue = realloc(retValue, strlen(retValue) + strlen(rteString) + 2);
            strcat(retValue, rteString);
            strcat(retValue, ",");
            free(rteString);
        }
        /* Remove comma on last route */
        retValue[strlen(retValue) - 1] = '\0';
    }
    strcat(retValue, "]");
    return retValue;
}

char* trackListToJSON(const List *list)
{
    char *retValue = calloc(5, sizeof(char));
    strcpy(retValue, "[");

    List *ls = (List *)list;
    if (list != NULL && getLength(ls) > 0)
    {
        ListIterator tracks = createIterator(ls);
        Track *trk;
        while ((trk = (Track *)nextElement(&tracks)) != NULL)
        {
            char *trkString = trackToJSON(trk);
            retValue = realloc(retValue, strlen(retValue) + strlen(trkString) + 2);
            strcat(retValue, trkString);
            strcat(retValue, ",");
            free(trkString);
        }
        /* Remove comma on last track */
        if (strlen(retValue) > 0)
        {
            retValue[strlen(retValue) - 1] = '\0';
        }
    }
    strcat(retValue, "]");
    return retValue;
}

char* waypointListToJSON(char *filename, int num)
{
    char *retValue = calloc(5, sizeof(char));
    strcpy(retValue, "[");

    GPXdoc *doc = createGPXdoc(filename);
    if (doc != NULL)
    {
        ListIterator routes = createIterator(doc->routes);
        Route *rte = NULL;
        for (int i = 0; i <= num; i++)
        {
            rte = (Route *)nextElement(&routes);
        }

        if (rte != NULL && rte->waypoints != NULL && getLength(rte->waypoints) > 0)
        {
            ListIterator wpts = createIterator(rte->waypoints);
            Waypoint *wpt;
            int i = 0;
            while ((wpt = (Waypoint *)nextElement(&wpts)) != NULL)
            {
                char *wptString = waypointToJSON(wpt, i);
                retValue = realloc(retValue, strlen(retValue) + strlen(wptString) + 2);
                strcat(retValue, wptString);
                strcat(retValue, ",");
                free(wptString);
                i++;
            }
            /* Remove comma on last route */
            retValue[strlen(retValue) - 1] = '\0';
        }
    }
    strcat(retValue, "]");
    return retValue;
}

char* GPXtoJSON(const GPXdoc* gpx)
{
    char *retValue = calloc(3, sizeof(char));
    if (gpx != NULL)
    {
        retValue = realloc(retValue, 150 + strlen(gpx->creator));
        sprintf(retValue, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx->version, gpx->creator, getNumWaypoints(gpx), getNumRoutes(gpx), getNumTracks(gpx));
    } else
    {
        strcpy(retValue, "{}");
    }
    return retValue;
}

/* END OF A2 FUNCTIONS 
 *
 * START OF A2 BONUS FUNCTIONS */

void addWaypoint(Route *rt, Waypoint *pt)
{
    if (rt != NULL && pt != NULL)
    {
        insertBack(rt->waypoints, pt);
    }
}

void addRoute(GPXdoc* doc, Route* rt)
{
    if (doc != NULL && rt != NULL)
    {
        insertBack(doc->routes, rt);
    }
}

GPXdoc* JSONtoGPX(const char* gpxString)
{
    if (gpxString != NULL)
    {
        GPXdoc *doc = initializeGPXdoc();
        strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");
        char temp[strlen(gpxString)];
        strcpy(temp, gpxString);

        char *tok = strtok(temp, "{}:,\"");
        /* Get second element, ie string following "version:" */
        if (tok != NULL && (tok = strtok(NULL, "{}:,\"")) != NULL)
        {
            doc->version = atof(tok);

            tok = strtok(NULL, "{}:,\"");
            /* Get fourth element, ie string following "creator:" */
            if (tok != NULL && (tok = strtok(NULL, "{}:,\"")) != NULL)
            {
                strcpy(doc->creator, tok);
            }
        }
        return doc;
    }
    return NULL;
}

Waypoint* JSONtoWaypoint(const char* gpxString)
{
    if (gpxString != NULL)
    {
        Waypoint *wpt = calloc(1, sizeof(Waypoint));
        wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
        wpt->name = calloc(1, sizeof(char));

        char temp[strlen(gpxString)];
        strcpy(temp, gpxString);

        char *tok = strtok(temp, "{}:,\"");
        /* Get second element, ie string following "latitude:" */
        if (tok != NULL && (tok = strtok(NULL, "{}:,\"")) != NULL)
        {
            wpt->latitude = atof(tok);

            tok = strtok(NULL, "{}:,\"");
            /* Get fourth element, ie string following "longitude:" */
            if (tok != NULL && (tok = strtok(NULL, "{}:,\"")) != NULL)
            {
                wpt->longitude = atof(tok);
            }
        }
        return wpt;
    }
    return NULL;
}

Route* JSONtoRoute(const char* gpxString)
{
    if (gpxString != NULL)
    {
        Route *rte = calloc(1, sizeof(Route));
        rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
        rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
        rte->name = NULL;

        char temp[strlen(gpxString)];
        strcpy(temp, gpxString);

        char *tok = strtok(temp, "{}:,\"");
        /* Get second element, ie string following "name:" */
        if (tok != NULL && (tok = strtok(NULL, "{}:,\"")) != NULL)
        {
            rte->name = calloc(strlen(tok) + 1, sizeof(char));
            strcpy(rte->name, tok);
        }

        /* Initialize route name to empty string if it wasnt allocated */
        if (rte->name == NULL)
        {
            rte->name = calloc(1, sizeof(char));
        }
        return rte;
    }
    return NULL;
}

/* END OF A2 BONUS FUNCTIONS */

/* START OF A3 FUNCTIONS */

char* createGPXFileFromJSON(const char *gpxString, const char *filename)
{
    char *retValue = calloc(20, sizeof(char));
    if (gpxString != NULL)
    {
        GPXdoc *doc = JSONtoGPX(gpxString);
        if (!validateGPXDoc(doc, "gpx.xsd") || !writeGPXdoc(doc, (char *)filename))
        {
            strcpy(retValue, "{\"status\":false}");
        } else
        {
            strcpy(retValue, "{\"status\":true}");
        }
        deleteGPXdoc(doc);
    } else
    {
        strcpy(retValue, "{\"status\":false}");
    }
    return retValue;
}

char* createGPXJSONFromFile(const char *filename)
{
    GPXdoc *doc = createValidGPXdoc((char *)filename, "gpx.xsd");
    char *retValue = GPXtoJSON(doc);
    deleteGPXdoc(doc);
    return retValue;
}

char* getGPXComponents(char *filename)
{
    GPXdoc *doc = createGPXdoc(filename);
    if (doc == NULL)
    {
        char *ret = calloc(20, sizeof(char));
        strcpy(ret, "{\"status\":false}");
        return ret;
    }
    char *str = routeListToJSON(doc->routes);
    char *strTwo = trackListToJSON(doc->tracks);
    char *ret = calloc(strlen(str) + strlen(strTwo) + 50, sizeof(char));
    sprintf(ret, "{\"routes\":%s,\"tracks\":%s,\"status\":true}", str, strTwo);
    deleteGPXdoc(doc);
    free(str);
    free(strTwo);
    return ret;
}

char* toStringCustomJSON(List * list){
	ListIterator iter = createIterator(list);
	char* str;
		
	str = (char*)malloc(sizeof(char));
	strcpy(str, "[");
	
	void* elem = nextElement(&iter);
    if (elem != NULL)
    {
        char* currDescr = list->printData(elem);
        int newLen = strlen(str)+50+strlen(currDescr);
        str = (char*)realloc(str, newLen);
        strcat(str, currDescr);

        while((elem = nextElement(&iter)) != NULL){
            currDescr = list->printData(elem);
            newLen = strlen(str)+50+strlen(currDescr);
            str = (char*)realloc(str, newLen);
            strcat(str, ",");
            strcat(str, currDescr);
            
            free(currDescr);
        }
    }
	
    strcat(str, "]");
	return str;
}

char* gpxDataListToJSON(char *filename, int isRoute, int component)
{
    GPXdoc *doc = createGPXdoc(filename);
    if (doc == NULL)
    {
        char *ret = calloc(20, sizeof(char));
        strcpy(ret, "{\"status\":false}");
        return ret;
    }
    if (isRoute)
    {
        Route *rte = NULL;
        ListIterator routes = createIterator(doc->routes);
        for (int i = 0; i < component; i++) 
        {
            rte = (Route *)nextElement(&routes);
        }
        if (rte == NULL)
        {
            char *ret = calloc(20, sizeof(char));
            strcpy(ret, "{\"status\":false}");
            deleteGPXdoc(doc);
            return ret;
        }
        char *dataString = toStringCustomJSON(rte->otherData);
        char *ret = calloc(strlen(dataString) + 40, sizeof(char));
        sprintf(ret, "{\"otherData\":%s,\"status\":true}", dataString);
        deleteGPXdoc(doc);
        free(dataString);
        return ret;
    } else
    {
        Track *trk = NULL;
        ListIterator tracks = createIterator(doc->tracks);
        for (int i = 0; i < component; i++) 
        {
            trk = (Track *)nextElement(&tracks);
        }
        if (trk == NULL)
        {
            char *ret = calloc(20, sizeof(char));
            strcpy(ret, "{\"status\":false}");
            deleteGPXdoc(doc);
            return ret;
        }

        char *dataString = toStringCustomJSON(trk->otherData);
        char *ret = calloc(strlen(dataString) + 40, sizeof(char));
        sprintf(ret, "{\"otherData\":%s,\"status\":true}", dataString);
        deleteGPXdoc(doc);
        free(dataString);
        return ret;
    }
}

char* renameComponent(char *filename, char *newName, int isRoute, int component)
{
    GPXdoc *doc = createGPXdoc(filename);
    char *ret = calloc(20, sizeof(char));
    if (doc == NULL)
    {
        strcpy(ret, "{\"status\":false}");
        return ret;
    }
    if (isRoute)
    {
        Route *rte = NULL;
        ListIterator routes = createIterator(doc->routes);
        for (int i = 0; i < component; i++) 
        {
            rte = (Route *)nextElement(&routes);
        }

        if (rte == NULL)
        {
            strcpy(ret, "{\"status\":false}");
        } else 
        {
            rte->name = realloc(rte->name, strlen(newName) + 1);
            strcpy(rte->name, newName);
        }
    } else
    {
        Track *trk = NULL;
        ListIterator tracks = createIterator(doc->tracks);
        for (int i = 0; i < component; i++) 
        {
            trk = (Track *)nextElement(&tracks);
        }

        if (trk == NULL)
        {
            strcpy(ret, "{\"status\":false}");
        } else 
        {
            trk->name = realloc(trk->name, strlen(newName) + 1);
            strcpy(trk->name, newName);
        }
    }
    if (!writeGPXdoc(doc, filename))
    {
        strcpy(ret, "{\"status\":false}");
    } else
    {
        strcpy(ret, "{\"status\":true}");
    }
    deleteGPXdoc(doc);
    return ret;
}

char* createRoute(char *filename, char *name, char *jsonStr)
{
    GPXdoc *doc = createGPXdoc(filename);
    char *ret = calloc(20, sizeof(char));
    if (doc == NULL)
    {
        strcpy(ret, "{\"status\":false}");
        return ret;
    }
    Route *rte = calloc(1, sizeof(Route));
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    rte->name = calloc(strlen(name) + 1, sizeof(char));
    strcpy(rte->name, name);

    jsonStr[strlen(jsonStr)-1] = '\0';
    char *tok = jsonStr;
    char *tokTwo = strchr(jsonStr, '}');
    while (tokTwo != NULL)
    {
        *tokTwo = '\0';
        char temp[strlen(tok)+5];
        sprintf(temp, "%s}", tok+1);
        Waypoint *wpt = JSONtoWaypoint(temp);
        if (wpt != NULL)
        {
            insertBack(rte->waypoints, wpt);
        }
        tokTwo++;
        tok = tokTwo;
        tokTwo = strchr(tokTwo, '}');
    }

    insertBack(doc->routes, rte);
    if (!validateGPXDoc(doc, "gpx.xsd") || !writeGPXdoc(doc, filename))
    {
        strcpy(ret, "{\"status\":false}");
    } else
    {
        strcpy(ret, "{\"status\":true}");
    }
    deleteGPXdoc(doc);
    return ret;
}

char* getPathBetween(char *filename, float sourceLat, float sourceLon, float destLat, float destLon, float delta)
{
    GPXdoc *doc = createGPXdoc(filename);
    if (doc == NULL)
    {
        char *ret = calloc(20, sizeof(char));
        strcpy(ret, "{\"status\":false}");
        return ret;
    }
    List *rtes = getRoutesBetween(doc, sourceLat, sourceLon, destLat, destLon, delta);
    List *trks = getTracksBetween(doc, sourceLat, sourceLon, destLat, destLon, delta);
    char *str = routeListToJSON(rtes);
    char *strTwo = trackListToJSON(trks);
    char *ret = calloc(strlen(str) + strlen(strTwo) + 50, sizeof(char));
    sprintf(ret, "{\"routes\":%s,\"tracks\":%s,\"status\":true}", str, strTwo);
    deleteGPXdoc(doc);
    free(str);
    free(strTwo);
    return ret;
}

/* END OF A3 FUNCTIONS */