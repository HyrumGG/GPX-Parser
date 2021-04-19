#include "GPXHelpers.h"

GPXdoc *initializeGPXdoc()
{
    GPXdoc *doc = calloc(1, sizeof(GPXdoc));
    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    doc->creator = calloc(256, sizeof(char));
    return doc;
}

void removeWhitespace(char *str)
{
    int i = strlen(str) - 1;
    while (str[i] == '\n' || str[i] == ' ')
    {
        i--;
    }
    str[i + 1] = '\0';
}

GPXData *parseGPXData(xmlNode *node)
{
    GPXData *data = calloc(1, sizeof(GPXData) + 256 * sizeof(char));
    
    if (node->name != NULL && node->children != NULL && node->children->content != NULL)
    {
        if (node->name[0] != '\0' && node->children->content[0] != '\0')
        {
            data = realloc(data, sizeof(GPXData) + (strlen((char *)(node->children->content)) + 1) * sizeof(char));
            strcpy(data->name, (char *)(node->name));
            removeWhitespace(data->name);
            strcat(data->value, (char *)(node->children->content));
            removeWhitespace(data->value);
            return data;
        }
    }
    deleteGpxData(data);
    return NULL;
}

Waypoint *parseWaypoint(xmlNode *node)
{
    Waypoint *wpt = calloc(1, sizeof(Waypoint));
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    wpt->name = calloc(256, sizeof(char));

    int latPresent = 0;
    int lonPresent = 0;
        
    xmlAttr *attr = node->properties;
    while (attr != NULL)
    {
        if (attr->name != NULL)
        {
            if(strcmp((char *)(attr->name), "lat") == 0)
            {
                xmlNode *val = attr->children;
                if (val != NULL && val->content != NULL)
                {
                    wpt->latitude = atof((char *)(val->content));
                    latPresent = 1;
                }
            } else if (strcmp((char *)(attr->name), "lon") == 0)
            {
                xmlNode *val = attr->children;
                if (val != NULL && val->content != NULL)
                {
                    wpt->longitude = atof((char *)(val->content));
                    lonPresent = 1;
                }
            }
        }
        attr = attr->next;
    }

    if (!latPresent || !lonPresent)
    {
        deleteWaypoint(wpt);
        return NULL;
    }

    xmlNode *cur = node->children;
    while (cur != NULL)
    {
        if (cur->type == XML_ELEMENT_NODE && cur->name != NULL) 
        {
            if(strcmp((char *)(cur->name), "name") == 0)
            {
                xmlNode *val = cur->children;
                if (val != NULL && val->content != NULL)
                {
                    wpt->name = realloc(wpt->name, (strlen((char *)(val->content)) + 1) * sizeof(char));
                    strcat(wpt->name, (char *)(val->content));
                    removeWhitespace(wpt->name);
                }
            } else
            {
                GPXData *data = parseGPXData(cur);
                if (data == NULL)
                {
                    deleteWaypoint(wpt);
                    return NULL;
                }
                insertBack(wpt->otherData, data);
            }
        }
        cur = cur->next;
    }
    return wpt;
}

TrackSegment *parseTrackSegment(xmlNode *node)
{
    TrackSegment *trkseg = calloc(1, sizeof(TrackSegment));
    trkseg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    
    xmlNode *cur = node->children;
    while (cur != NULL)
    {
        if (cur->type == XML_ELEMENT_NODE && cur->name != NULL) 
        {
            if (strcmp((char *)(cur->name), "trkpt") == 0)
            {
                /*QUESTION: do we have a list of all waypoints in gpxdoc, or can we have
                waypoints in a route not be present in the gpxdoc list */
                Waypoint *wpt = parseWaypoint(cur);
                if (wpt == NULL)
                {
                    deleteTrackSegment(trkseg);
                    return 0;
                }
                insertBack(trkseg->waypoints, wpt);
            }
        }
        cur = cur->next;
    }
    return trkseg;
}

Route *parseRoute(xmlNode *node)
{
    Route *rte = calloc(1, sizeof(Route));
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    rte->name = calloc(256, sizeof(char));

    xmlNode *cur = node->children;
    while (cur != NULL)
    {
        if (cur->type == XML_ELEMENT_NODE && cur->name != NULL) 
        {
            if(strcmp((char *)(cur->name), "name") == 0)
            {
                xmlNode *val = cur->children;
                if (val != NULL && val->content != NULL)
                {
                    rte->name = realloc(rte->name, (strlen((char *)(val->content)) + 1) * sizeof(char));
                    strcat(rte->name, (char *)(val->content));
                    removeWhitespace(rte->name);
                }
            } else if (strcmp((char *)(cur->name), "rtept") == 0)
            {
                /*QUESTION: do we have a list of all waypoints in gpxdoc, or do can we have
                waypoints in a route not be present in the gpxdoc list */
                Waypoint *wpt = parseWaypoint(cur);
                if (wpt == NULL)
                {
                    deleteRoute(rte);
                    return 0;
                }
                insertBack(rte->waypoints, wpt);
            } else
            {
                GPXData *data = parseGPXData(cur);
                if (data == NULL)
                {
                    deleteRoute(rte);
                    return NULL;
                }
                insertBack(rte->otherData, data);
            }
        }
        cur = cur->next;
    }
    return rte;
}

Track *parseTrack(xmlNode *node)
{
    Track *trk = calloc(1, sizeof(Track));
    trk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    trk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    trk->name = calloc(256, sizeof(char));

    xmlNode *cur = node->children;
    while (cur != NULL)
    {
        if (cur->type == XML_ELEMENT_NODE && cur->name != NULL) 
        {
            if(strcmp((char *)(cur->name), "name") == 0)
            {
                xmlNode *val = cur->children;
                if (val != NULL && val->content != NULL)
                {
                    trk->name = realloc(trk->name, (strlen((char *)(val->content)) + 1) * sizeof(char));
                    strcat(trk->name, (char *)(val->content));
                    removeWhitespace(trk->name);
                }
            } else if (strcmp((char *)(cur->name), "trkseg") == 0)
            {
                TrackSegment *trkseg = parseTrackSegment(cur);
                if (trkseg == NULL)
                {
                    deleteTrack(trk);
                    return NULL;
                }
                insertBack(trk->segments, trkseg);
            } else
            {
                GPXData *data = parseGPXData(cur);
                if (data == NULL)
                {
                    deleteTrack(trk);
                    return NULL;
                }
                insertBack(trk->otherData, data);
            }
        }
        cur = cur->next;
    }
    return trk;
}

int getGPXdocAttributes(GPXdoc *doc, xmlNode *root)
{
    int foundCreator = 0;
    int foundVersion = 0;

    if (root->ns == NULL || root->ns->href == NULL) 
    {
        return 0;
    }
    strcpy(doc->namespace, (char *)(root->ns->href));

    xmlAttr *attr = root->properties;
    while (attr != NULL)
    {
        if (attr->name != NULL)
        {
            if(strcmp((char *)(attr->name), "creator") == 0)
            {
                xmlNode *val = attr->children;
                if (val != NULL && val->content != NULL)
                {
                    doc->creator = realloc(doc->creator, (strlen((char *)(val->content)) + 1) * sizeof(char));
                    strcat(doc->creator, (char *)(val->content));
                    foundCreator = 1;
                }
            } else if (strcmp((char *)(attr->name), "version") == 0)
            {
                xmlNode *val = attr->children;
                if (val != NULL && val->content != NULL)
                {
                    doc->version = atof((char *)(val->content));
                    foundVersion = 1;
                }
            }
        }
        attr = attr->next;
    }

    if (!foundVersion || !foundCreator)
    {
        return 0;
    }
    return 1;
}

int generateGPXdoc(GPXdoc *doc, xmlNode *root)
{
    if (!getGPXdocAttributes(doc, root))
    {
        return 0;
    }

    xmlNode *cur = root->children;
    while (cur != NULL)
    {
        if (cur->type == XML_ELEMENT_NODE && cur->name != NULL) 
        {
            if (strcmp((char *)(cur->name), "wpt") == 0)
            {
                Waypoint *wpt = parseWaypoint(cur);
                if (wpt == NULL)
                {
                    return 0;
                }
                insertBack(doc->waypoints, wpt);
            } else if (strcmp((char *)(cur->name), "rte") == 0)
            {
                Route *rte = parseRoute(cur);
                if (rte == NULL)
                {
                    return 0;
                }
                insertBack(doc->routes, rte);
            } else if (strcmp((char *)(cur->name), "trk") == 0)
            {
                Track *trk = parseTrack(cur);
                if (trk == NULL)
                {
                    return 0;
                }
                insertBack(doc->tracks, trk);
            }
        }
        cur = cur->next;
    }
    return 1;
}

int getGPXDataInWaypoint(ListIterator wpts)
{
    int count = 0;
    void *ele;
    while ((ele = nextElement(&wpts)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)ele;
        count += getLength(wpt->otherData);
        if (strcmp(wpt->name, "") != 0) {
            count++;
        }
    }
    return count;
}

Waypoint *findWaypointInList(ListIterator wpts, char *name)
{
    void *ele;
    while ((ele = nextElement(&wpts)) != NULL)
    {
        Waypoint *wpt = (Waypoint *)ele;
        if (strcmp(wpt->name, name) == 0)
        {
            return wpt;
        }
    }
    return NULL;
}

int nodeAfterName(char *nodeStr)
{
    if (strcmp(nodeStr, "cmt") == 0 || strcmp(nodeStr, "desc") == 0 || strcmp(nodeStr, "src") == 0 || strcmp(nodeStr, "link") == 0 || strcmp(nodeStr, "sym") == 0 || strcmp(nodeStr, "type") == 0 || strcmp(nodeStr, "fix") == 0 || strcmp(nodeStr, "sat") == 0 || strcmp(nodeStr, "hdop") == 0 || strcmp(nodeStr, "vdop") == 0 || strcmp(nodeStr, "pdop") == 0 || strcmp(nodeStr, "ageofdgpsdata") == 0 || strcmp(nodeStr, "dgpsid") == 0 || strcmp(nodeStr, "extensions") == 0)
    {
        return 1;
    }
    return 0;
}

void waypointConvert(xmlNode *node, Waypoint *wpt, int *constraintsValid)
{
    /* Convert lat and lon to strings and add them to waypoint node */
    char lat[20], lon[20];
    sprintf(lat, "%f", wpt->latitude);
    xmlNewProp(node, (const xmlChar *)"lat", (const xmlChar *)lat);
    sprintf(lon, "%f", wpt->longitude);
    xmlNewProp(node, (const xmlChar *)"lon", (const xmlChar *)lon);

    int nameConverted = 0;

    /* Add other data to waypoint node */
    if (wpt->otherData != NULL)
    {
        ListIterator otherData = createIterator(wpt->otherData);
        GPXData *data;
        while ((data = (GPXData *)nextElement(&otherData)) != NULL)
        {
            if (data->name != NULL && strcmp(data->name, "") != 0 && data->value != NULL && strcmp(data->value, "") != 0)
            {
                /* Fetch and validate name node */
                if (nameConverted == 0 && nodeAfterName(data->name))
                {
                    if (wpt->name != NULL)
                    {
                        if (strcmp(wpt->name, "") != 0)
                        {
                            xmlNewChild(node, NULL, (const xmlChar *)"name", (const xmlChar *)wpt->name);
                            nameConverted = 1;
                        }
                    } else
                    {
                        *constraintsValid = 0;
                    }
                }
                xmlNewChild(node, NULL, (const xmlChar *)data->name, (const xmlChar *)data->value);
            } else
            {
                *constraintsValid = 0;
            }
        }
        /* Fetch and validate name node */
        if (nameConverted == 0)
        {
            if (wpt->name != NULL)
            {
                if (strcmp(wpt->name, "") != 0)
                {
                    xmlNewChild(node, NULL, (const xmlChar *)"name", (const xmlChar *)wpt->name);
                }
            } else
            {
                *constraintsValid = 0;
            }
        }
    } else
    {
        *constraintsValid = 0;
    }
}

void routeConvert(xmlNode *node, Route *rte, int *constraintsValid)
{
    /* Fetch and validate route name */
    if (rte->name != NULL)
    {
        if (strcmp(rte->name, "") != 0)
        {
            xmlNewChild(node, NULL, (const xmlChar *)"name", (const xmlChar *)rte->name);
        }
    } else
    {
        *constraintsValid = 0;
    }

    /* Add other data to waypoint node */
    if (rte->otherData != NULL)
    {
        ListIterator otherData = createIterator(rte->otherData);
        GPXData *data;
        while ((data = (GPXData *)nextElement(&otherData)) != NULL)
        {
            if (data->name != NULL && strcmp(data->name, "") != 0 && data->value != NULL && strcmp(data->value, "") != 0)
            {
                xmlNewChild(node, NULL, (const xmlChar *)data->name, (const xmlChar *)data->value);
            } else
            {
                *constraintsValid = 0;
            }
        }
    } else
    {
        *constraintsValid = 0;
    }

    /* Add all waypoint children to route */
    if (rte->waypoints != NULL)
    {
        ListIterator wpts = createIterator(rte->waypoints);
        Waypoint *wpt;
        while ((wpt = (Waypoint *)nextElement(&wpts)) != NULL)
        {
            xmlNode *child = xmlNewChild(node, NULL, (const xmlChar *)"rtept", NULL);
            waypointConvert(child, wpt, constraintsValid);
        }
    } else
    {
        *constraintsValid = 0;
    }
}

void trackConvert(xmlNode *node, Track *trk, int *constraintsValid)
{
    /* Fetch and validate route name */
    if (trk->name != NULL)
    {
        if (strcmp(trk->name, "") != 0)
        {
            xmlNewChild(node, NULL, (const xmlChar *)"name", (const xmlChar *)trk->name);
        }
    } else
    {
        *constraintsValid = 0;
    }

    /* Add other data to waypoint node */
    if (trk->otherData != NULL)
    {
        ListIterator otherData = createIterator(trk->otherData);
        GPXData *data;
        while ((data = (GPXData *)nextElement(&otherData)) != NULL)
        {
            if (data->name != NULL && strcmp(data->name, "") != 0 && data->value != NULL && strcmp(data->value, "") != 0)
            {
                xmlNewChild(node, NULL, (const xmlChar *)data->name, (const xmlChar *)data->value);
            } else
            {
                *constraintsValid = 0;
            }
        }
    } else
    {
        *constraintsValid = 0;
    }

    /* Add track segments */
    if (trk->segments != NULL)
    {
        ListIterator segments = createIterator(trk->segments);
        TrackSegment *trkseg;
        while ((trkseg = (TrackSegment *)nextElement(&segments)) != NULL)
        {
            xmlNode *segNode = xmlNewChild(node, NULL, (const xmlChar *)"trkseg", NULL);
            /* Add all waypoint children to route */
            if (trkseg->waypoints != NULL)
            {
                ListIterator wpts = createIterator(trkseg->waypoints);
                Waypoint *wpt;
                while ((wpt = (Waypoint *)nextElement(&wpts)) != NULL)
                {
                    xmlNode *child = xmlNewChild(segNode, NULL, (const xmlChar *)"trkpt", NULL);
                    waypointConvert(child, wpt, constraintsValid);
                }
            } else
            {
                *constraintsValid = 0;
            }
        }
    } else
    {
        *constraintsValid = 0;
    }
}

xmlDoc *xmlDocConvert(GPXdoc *doc, int *constraintsValid)
{
    xmlDoc *newDoc = NULL;
    xmlNode *node = NULL;

    newDoc = xmlNewDoc((const xmlChar *)"1.0");
    node = xmlNewNode(NULL, (const xmlChar *)"gpx");
    xmlDocSetRootElement(newDoc, node);

    /* Get and set the namespace and attributes of gpx element of the doc */
    if (doc->namespace != NULL && strcmp(doc->namespace, "") != 0)
    {
        xmlSetNs(node, xmlNewNs(node, (const xmlChar *)doc->namespace, NULL));
    } else
    {
        *constraintsValid = 0;
    }
    char version[20];
    sprintf(version, "%.1f", doc->version);
    xmlNewProp(node, (const xmlChar *)"version", (const xmlChar *)version);
    if (doc->creator != NULL && strcmp(doc->creator, "") != 0)
    {
        xmlNewProp(node, (const xmlChar *)"creator", (const xmlChar *)doc->creator);
    } else
    {
        printf("test2\n");
        *constraintsValid = 0;
    }
    
    /* Add all waypoint children to gpx */
    if (doc->waypoints != NULL)
    {
        ListIterator wpts = createIterator(doc->waypoints);
        Waypoint *wpt;
        while ((wpt = (Waypoint *)nextElement(&wpts)) != NULL)
        {
            xmlNode *child = xmlNewChild(node, NULL, (const xmlChar *)"wpt", NULL);
            waypointConvert(child, wpt, constraintsValid);
        }
    } else
    {
        printf("test3\n");
        *constraintsValid = 0;
    }

    /* Add all route children to gpx */
    if (doc->routes != NULL)
    {
        ListIterator rtes = createIterator(doc->routes);
        Route *rte;
        while ((rte = (Route *)nextElement(&rtes)) != NULL)
        {
            xmlNode *child = xmlNewChild(node, NULL, (const xmlChar *)"rte", NULL);
            routeConvert(child, rte, constraintsValid);
        }
    } else
    {
        printf("test4\n");
        *constraintsValid = 0;
    }

    /* Add all track children to gpx */
    if (doc->tracks != NULL)
    {
        ListIterator trks = createIterator(doc->tracks);
        Track *trk;
        while ((trk = (Track *)nextElement(&trks)) != NULL)
        {
            xmlNode *child = xmlNewChild(node, NULL, (const xmlChar *)"trk", NULL);
            trackConvert(child, trk, constraintsValid);
        }
    } else
    {
        printf("test5\n");
        *constraintsValid = 0;
    }
    return newDoc;
}

/* Validation code below is referenced from http://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html */
bool validateXMLDoc(xmlDoc *doc, char *gpxSchemaFile)
{
    xmlSchema *schema = NULL;
    xmlSchemaParserCtxt *context = NULL;
    xmlSchemaValidCtxt *validContext = NULL;

    xmlLineNumbersDefault(1);

    context = xmlSchemaNewParserCtxt(gpxSchemaFile);
	xmlSchemaSetParserErrors(context, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(context);
	xmlSchemaFreeParserCtxt(context);

    /* Create validation context */
    validContext = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(validContext, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    
    /* Validate the gpx doc */
    int retValue = xmlSchemaValidateDoc(validContext, doc);
    xmlSchemaFreeValidCtxt(validContext);

    /* free document if invalid */
    if (retValue != 0)
    {
        xmlFreeDoc(doc);
    }

    if(schema != NULL)
    {
    	xmlSchemaFree(schema);
    }

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    /* returns true (1) if the return value of xmlSchemaValidateDoc() was 0 otherwise returns false (0) */
    return retValue == 0;
}

float calculateDistance(Waypoint *wpt, Waypoint *wptTwo)
{
    if (wpt != NULL && wptTwo != NULL)
    {
        int r = 6371e3;
        /* Convert coordinates into radians */
        float radsLat1 = wpt->latitude * M_PI/180;
        float radsLat2 = wptTwo->latitude * M_PI/180;
        float deltaLatRads = (wptTwo->latitude - wpt->latitude) * M_PI/180;
        float deltaLonRads = (wptTwo->longitude - wpt->longitude) * M_PI/180;
        float a = pow(sin(deltaLatRads/2), 2) + cos(radsLat1) * cos(radsLat2) * pow(sin(deltaLonRads/2), 2);
        float c = 2 * atan2(sqrtf(a), sqrtf(1-a));
        return r * c;
    }
    return 0;
}

void dummy(void *data)
{
    return;
}
