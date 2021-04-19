#ifndef HELPER_H
#define HELPER_H

#include "GPXParser.h"

GPXdoc *initializeGPXdoc();
int generateGPXdoc(GPXdoc *doc, xmlNode *root);
int getGPXdocAttributes(GPXdoc *doc, xmlNode *root);
Waypoint *parseWaypoint(xmlNode *node);
Route *parseRoute(xmlNode *node);
Track *parseTrack(xmlNode *node);
TrackSegment *parseTrackSegment(xmlNode *node);
GPXData *parseGPXData(xmlNode *node);
void removeWhitespace(char *str);
int getGPXDataInWaypoint(ListIterator wpts);
Waypoint *findWaypointInList(ListIterator wpts, char *name);
int nodeAfterName(char *nodeStr);
void waypointConvert(xmlNode *node, Waypoint *wpt, int *constraintsValid);
void routeConvert(xmlNode *node, Route *rte, int *constraintsValid);
void trackConvert(xmlNode *node, Track *trk, int *constraintsValid);
xmlDoc *xmlDocConvert(GPXdoc *doc, int *constraintsValid);
bool validateXMLDoc(xmlDoc *doc, char *gpxSchemaFile);
float calculateDistance(Waypoint *wpt, Waypoint *wptTwo);
void dummy(void *data);

#endif