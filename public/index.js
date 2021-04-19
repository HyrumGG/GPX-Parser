let file_log_table;
let file_log_table_body;
let view_panel_table;
let view_panel_table_body;
let view_panel_drop_down;
let route_drop_down;
let show_other_drop_down;
let rename_drop_down;
let create_filename;
let create_creator;
let create_version;
let lastValidFileViewPanel = "";
let lastStartPoint = {};
let lastEndPoint = {};
let lastDelta = -1;
let waypoints = [];
let path_between_table;
let path_between_table_body;
let loggedIn = false;
let files = 0;
let route_query_dropdown;
let point_route_query_dropdown;
let route_longest_query_dropdown;
let point_route_query_table;
let point_file_query_table;
let point_file_query_dropdown;
let route_query_table;
let route_longest_query_table;

// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(function () {
    file_log_table = document.getElementById('file-log-table');
    route_query_table = document.getElementById('route-query-table').lastElementChild;
    point_route_query_table = document.getElementById('point-route-query-table').lastElementChild;
    point_file_query_table = document.getElementById('point-file-query-table').lastElementChild;
    route_longest_query_table = document.getElementById('route-longest-query-table').lastElementChild;
    view_panel_table = document.getElementById('view-panel-table');
    path_between_table = document.getElementById('path-between-table');
    file_log_table_body = file_log_table.lastElementChild;
    view_panel_table_body = view_panel_table.lastElementChild;
    path_between_table_body = path_between_table.lastElementChild;
    view_panel_drop_down = document.getElementById('view-panel-drop-down');
    route_drop_down = document.getElementById('route-drop-down');
    show_other_drop_down = document.getElementById('show-other-drop-down');
    rename_drop_down = document.getElementById('rename-drop-down');
    create_filename = document.getElementById('create-filename');
    create_creator = document.getElementById('create-creator');
    create_version = document.getElementById('create-version');
    route_query_dropdown = document.getElementById('route-query-dropdown');
    point_route_query_dropdown = document.getElementById('point-route-query-dropdown');
    route_longest_query_dropdown = document.getElementById('route-longest-query-dropdown');
    point_file_query_dropdown = document.getElementById('point-file-query-dropdown');

    document.getElementById('display-status').disabled = true;
    document.getElementById('store-all').disabled = true;
    document.getElementById('clear-all').disabled = true;
    document.getElementById('query1').disabled = true;
    document.getElementById('query3').disabled = true;
    document.getElementById('query4').disabled = true;
    document.getElementById('query5').disabled = true;

    updateFileLog();

    $('#uploadForm').submit(function (e) {
        e.preventDefault();
        let form = new FormData(this);
        let filename = form.get('uploadFile').name;
        if (filename === undefined) {
            alert('Please select a gpx file');
        } else {
            jQuery.ajax({
                type: 'post',
                url: '/upload',
                data: form,
                contentType: false,
                cache: false,
                processData: false,
                success: function () {
                    console.log('Uploaded file - checking validity');
                    addToFileLog(filename);
                },
                error: function (error) {
                    if (error.status === 400) {
                        console.log("file exists");
                        alert(filename + " already exists");
                    } else {
                        console.log("Error uploading");
                        alert("Error occured uploading " + filename);
                    }
                }
            });
        }
    });

    $('#view-panel-drop-down').change(function (e) {
        let name = this.value;
        if (name !== 'No file selected') {
            jQuery.ajax({
                type: 'get',
                dataType: 'json',
                url: '/showComponents',
                data: {
                    filename: name
                },
                success: function (data) {
                    if (data.status) {
                        lastValidFileViewPanel = name;
                        view_panel_table_body.innerHTML = "";
                        rename_drop_down.innerHTML = "<option>Component to rename</option>";
                        show_other_drop_down.innerHTML = "<option>Show Other Data of</option>";
                        for (let i in data.routes) {
                            data.routes[i]['component'] = "Route " + (+i + +1);
                            addComponentToView(data.routes[i]);
                        }
                        for (let i in data.tracks) {
                            data.tracks[i]['component'] = "Track " + (+i + +1);
                            addComponentToView(data.tracks[i]);
                        }
                        alert('Loaded view table with components in ' + name);
                        console.log('Loaded components');
                    } else {
                        console.log('Error reading file');
                        alert('Error reading ' + name);
                    }
                },
                error: function (error) {
                    alert('Error occured making request');
                    console.log(error);
                }
            });
        } else {
            console.log('Invalid Option');
            alert('Invalid Option - Please select a file');
        }
    });

    $('#show-other-drop-down').change(function (e) {
        let otherComponent = this.value;
        if (otherComponent !== 'Show Other Data of') {
            jQuery.ajax({
                type: 'get',
                dataType: 'json',
                url: '/showOtherData',
                data: {
                    filename: lastValidFileViewPanel,
                    component: otherComponent
                },
                success: function (data) {
                    if (data.status) {
                        if (data.otherData.length == 0) {
                            console.log('Component has no other data');
                            alert(otherComponent + ' does not have any other data');
                        } else {
                            let componentStr = "Other Data of " + otherComponent + ":\n";
                            for (let i in data.otherData) {
                                componentStr += (+i + +1) + '. ' + data.otherData[i].name + ': ' + data.otherData[i].value + '\n';
                            }
                            alert(componentStr);
                            console.log('Showing other data');
                        }
                    } else {
                        console.log('Error getting data component');
                        alert('Error getting data of ' + otherComponent);
                    }
                },
                error: function (error) {
                    alert('Error occured making request');
                    console.log(error);
                }
            });
        } else {
            console.log('Invalid Option');
            alert('Invalid Option - Please select a component');
        }
    });

    $('#view-panel-input').submit(function (e) {
        e.preventDefault();
        let rename = $('#rename-drop-down')[0].value;
        let newName = $('#rename-new-name')[0].value;
        if (rename !== 'Component to rename') {
            jQuery.ajax({
                type: 'get',
                dataType: 'json',
                url: '/renameComponent',
                data: {
                    filename: lastValidFileViewPanel,
                    component: rename,
                    name: newName
                },
                success: function (data) {
                    if (data.status) {
                        // Find component in table and rename its name to the new name given
                        for (let i = 0; i < view_panel_table_body.childNodes.length; i++) {
                            if (rename === view_panel_table_body.childNodes[i].firstChild.innerText) {
                                view_panel_table_body.childNodes[i].childNodes[1].innerText = newName;
                                break;
                            }
                        }
                        if (lastDelta != -1) {
                            updatePathBetween(false);
                        }
                        alert('Renamed ' + rename);
                        console.log('Renamed component');
                        rebuildTable(lastValidFileViewPanel);
                    } else {
                        console.log('Error renaming');
                        alert('Error renaming ' + rename);
                    }
                },
                error: function (error) {
                    alert('Error occured making request');
                    console.log(error);
                }
            });
        } else {
            console.log('Invalid Option');
            alert('Invalid Option - Please select a component');
        }
    });

    $('#create-gpx').submit(function (e) {
        e.preventDefault();
        if (create_creator.value.length == 0 || create_filename.value.length == 0 || create_version.value.length == 0) {
            console.log('Create-GPX: Missing Input');
            alert('Please input all fields before creating a GPX file');
        } else {
            if (create_version.value.length != 3) {
                console.log('Version invalid');
                alert('Error creating file ' + create_filename.value + ', attributes are invalid');
            } else {
                jQuery.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/createGPX',
                    data: {
                        filename: create_filename.value,
                        version: create_version.value,
                        creator: create_creator.value
                    },
                    success: function (data) {
                        if (data.stat) {
                            console.log('Created file - checking validity');
                            addToFileLog(create_filename.value);
                        } else {
                            console.log('Error creating file, could be invalid');
                            alert('Error creating file ' + create_filename.value + ', may be invalid');
                        }
                    },
                    error: function (error) {
                        alert('Error occured making request');
                        console.log(error);
                    }
                });
            }
        }
    });

    $('#waypoint-add').click(function (e) {
        if (!isNaN(this.parentElement.childNodes[3].value) && !isNaN(this.parentElement.childNodes[5].value)) {
            if (parseFloat(this.parentElement.childNodes[3].value) >= -90 && parseFloat(this.parentElement.childNodes[3].value) <= 90 &&
                parseFloat(this.parentElement.childNodes[5].value) >= -180 && parseFloat(this.parentElement.childNodes[5].value) < 180.0) {
                waypoints.push({
                    'lat': parseFloat(this.parentElement.childNodes[3].value),
                    'lon': parseFloat(this.parentElement.childNodes[5].value)
                });
                alert('Added waypoint');
                console.log('Added waypoint');
            } else {
                alert('Latitude must be between -90 and 90, Longitude must be larger than -180 and smaller than 180');
                console.log('invalid coordinates');
            }
        } else {
            console.log('Invalid Option');
            alert('Please input waypoint coordinates');
        }
    });

    $('#route-add').submit(function (e) {
        e.preventDefault();
        let routeFile = this[0].value;
        let routeName = this[1].value;
        if (routeFile !== 'No file selected') {
            jQuery.ajax({
                type: 'get',
                dataType: 'json',
                url: '/addRoute',
                data: {
                    filename: routeFile,
                    name: routeName,
                    wpts: JSON.stringify(waypoints)
                },
                success: function (data) {
                    if (data.status) {
                        if (routeFile === lastValidFileViewPanel) {
                            updateComponents();
                        } else {
                            console.log('not updating component');
                        }
                        updateFileLogRoute(routeFile);
                        if (waypoints.length > 0 && lastDelta != -1) {
                            updatePathBetween(false);
                        }
                        alert('Added Route ' + routeName);
                        console.log('Added Route');
                        rebuildTable(routeFile);
                    } else {
                        console.log('Error adding route');
                        alert('Error adding route, may be invalid');
                    }
                    waypoints = [];
                },
                error: function (error) {
                    alert('Error occured making request');
                    console.log(error);
                }
            });
        } else {
            console.log('Invalid Option');
            alert('Please select a file to add a route');
        }
    });

    $('#path-between').submit(function (e) {
        e.preventDefault();
        if (this[0].value.length != 0 && this[1].value.length != 0 && this[2].value.length != 0 && this[3].value.length != 0 && this[4].value.length != 0 && !isNaN(this[4].value) && 
            !isNaN(this[0].value) && !isNaN(this[1].value) && !isNaN(this[2].value) && !isNaN(this[3].value)) {
            if (parseFloat(this[0].value) >= -90 && parseFloat(this[0].value) <= 90 && parseFloat(this[2].value) >= -90 && parseFloat(this[2].value) <= 90 &&
                parseFloat(this[1].value) >= -180 && parseFloat(this[1].value) < 180.0 && parseFloat(this[3].value) >= -180 && parseFloat(this[3].value) < 180.0) {
                lastStartPoint = {
                    'lat': this[0].value,
                    'lon': this[1].value
                };
                lastEndPoint = {
                    'lat': this[2].value,
                    'lon': this[3].value
                };
                lastDelta = this[4].value;
                if (lastDelta < 0) {
                    console.log('Please input a point accuracy above 0');
                    alert('Please input a point accuracy larger than or equal to 0');
                } else {
                    updatePathBetween(true);
                }
            } else {
                alert('Latitude must be between -90 and 90, Longitude must be larger than -180 and smaller than 180');
                console.log('invalid coordinates');
            }

        } else {
            console.log('Invalid Option');
            alert('Invalid Option - Please input all coordinate fields with numeric values');
        }
    });

    $('#login').submit(function (e) {
        e.preventDefault();
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/dblogin',
            data: {
                username: this[0].value,
                password: this[1].value,
                databaseName: this[2].value
            },
            success: function (data) {
                if (data.status) {
                    console.log('Logged in');
                    alert('Logged in');
                    loggedIn = true;
                    if (files != 0) {
                        document.getElementById('store-all').disabled = false;
                    }
                    document.getElementById('display-status').disabled = false;
                    document.getElementById('query1').disabled = false;
                    document.getElementById('query3').disabled = false;
                    document.getElementById('query4').disabled = false;
                    document.getElementById('query5').disabled = false;
                    getStatus();
                    updateDBDropdowns();
                } else {
                    console.log('Error logging in');
                    alert('Error logging into database, Please input verify input fields and try again');
                }
            },
            error: function (error) {
                alert('Error occured logging in, ensure details');
                console.log(error);
            }
        });
    });

    $('#store-all').click(function (e) {
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/storeFiles',
            success: function (data) {
                if (data.status) {
                    alert('Stored all files');
                    console.log('stored files');
                    getStatus();
                    updateDBDropdowns();
                } else {
                    console.log('Error storing all files, ensure login');
                    alert('Error storing all files, ensure login');
                }
            },
            error: function (error) {
                console.log(error);
                alert("Error occurred storing all files, ensure login");
            }
        });
    });

    $('#clear-all').click(function (e) {
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/clearData',
            success: function (data) {
                if (data.status) {
                    alert('Cleared All Data');
                    console.log('Cleared All Data');
                    getStatus();
                    updateDBDropdowns();
                    document.getElementById('clear-all').disabled = true;
                } else {
                    console.log('Error clearing data, ensure login');
                    alert('Error clearing data, ensure login');
                }
            },
            error: function (error) {
                console.log(error);
                alert("Error occurred clearing data, ensure login");
            }
        });
    });

    $('#display-status').click(function (e) {
        getStatus();
    });

    $('#route-query').submit(function (e) {
        e.preventDefault();
        let all = false;
        if ($('#route-query-dropdown')[0].value === 'All Files') {
            all = true;
        }
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/routeQuery',
            data: {
                filename: $('#route-query-dropdown')[0].value,
                allFiles: all,
                type: $('#route-query-sort')[0].value
            },
            success: function (data) {
                if (data.status) {
                    route_query_table.innerHTML = "";
                    for (let i in data.routes) {
                        let str = "";
                        str += '<tr><td>' + data.routes[i].filename + '</td>' + '<td>' + data.routes[i].route_id + '</td>' + '<td>' + data.routes[i].route_name + '</td>' + '<td>' + data.routes[i].route_len + 'm</td>'
                        + '<td>' + data.routes[i].gpx_id + '</td>';
                        str += '</tr>';
                        route_query_table.innerHTML += str;
                    }
                    alert('Executed Route Query');
                    console.log('Executed Route Query');
                } else {
                    console.log('Error executing route query, ensure login');
                    alert('Error executing route query, ensure login');
                }
            },
            error: function (error) {
                console.log(error);
                alert("Error executing route query, ensure login");
            }
        });
    });

    $('#point-route-query').submit(function (e) {
        e.preventDefault();
        if ($('#point-route-query-dropdown')[0].value !== 'No file selected') {
            jQuery.ajax({
                type: 'get',
                dataType: 'json',
                url: '/pointQuery',
                data: {
                    filename: $('#point-route-query-dropdown')[0].value,
                    rteName: this[0].value
                },
                success: function (data) {
                    if (data.status) {
                        point_route_query_table.innerHTML = "";
                        for (let i in data.points) {
                            let str = "";
                            str += '<tr><td>' + data.points[i].point_id + '</td>' + '<td>' + data.points[i].point_index + '</td>' + '<td>' + data.points[i].latitude + '</td>'
                            + '<td>' + data.points[i].longitude + '</td>' + '<td>' + data.points[i].point_name + '</td>' + '<td>' + data.points[i].route_id + '</td>';
                            str += '</tr>';
                            point_route_query_table.innerHTML += str;
                        }
                        alert('Executed Point Query');
                        console.log('Executed Point Query');
                    } else {
                        console.log('Error executing Point Query, ensure login');
                        alert('Error executing Point Query, ensure login');
                    }
                },
                error: function (error) {
                    console.log(error);
                    alert("Error executing Point Query, ensure login");
                }
            });
        } else {
            alert('Please select a file');
        }
    });

    $('#point-file-query').submit(function (e) {
        e.preventDefault();
        if ($('#point-file-query-dropdown')[0].value !== 'No file selected') {
            jQuery.ajax({
                type: 'get',
                dataType: 'json',
                url: '/pointFileQuery',
                data: {
                    filename: $('#point-file-query-dropdown')[0].value,
                    type: $('#point-query-sort')[0].value
                },
                success: function (data) {
                    if (data.status) {
                        point_file_query_table.innerHTML = "";
                        for (let i in data.points) {
                            let str = "";
                            str += '<tr><td>' + data.points[i].point_id + '</td>' + '<td>' + data.points[i].point_index + '</td>' + '<td>' + data.points[i].latitude + '</td>'
                            + '<td>' + data.points[i].longitude + '</td>' + '<td>' + data.points[i].point_name + '</td>' + '<td>' + data.points[i].route_id + '</td>'
                            + '<td>' + data.points[i].route_name + '</td>' + '<td>' + data.points[i].route_len + 'm</td>';
                            str += '</tr>';
                            point_file_query_table.innerHTML += str;
                        }
                        alert('Executed Point File Query');
                        console.log('Executed Point File Query');
                    } else {
                        console.log('Error executing Point File Query, ensure login');
                        alert('Error executing Point File Query, ensure login');
                    }
                },
                error: function (error) {
                    console.log(error);
                    alert("Error executing Point File Query, ensure login");
                }
            });
        } else {
            alert('Please select a file');
        }
    });

    $('#route-longest-query').submit(function (e) {
        e.preventDefault();
        if ($('#route-longest-query-dropdown')[0].value !== 'No file selected' && this[0].value.length != 0 && !isNaN(this[0].value) && this[0].value > 0) {
            jQuery.ajax({
                type: 'get',
                dataType: 'json',
                url: '/routeLongestQuery',
                data: {
                    filename: $('#route-longest-query-dropdown')[0].value,
                    size: this[0].value,
                    asc: this[1].value,
                    type: $('#route-longest-query-sort')[0].value
                },
                success: function (data) {
                    if (data.status) {
                        route_longest_query_table.innerHTML = "";
                        for (let i in data.routes) {
                            let str = "";
                            str += '<tr><td>' + $('#route-longest-query-dropdown')[0].value + '<td>' + data.routes[i].route_id + '</td>' + '<td>' + data.routes[i].route_name + '</td>' + '<td>' + data.routes[i].route_len + 'm</td>'
                            + '<td>' + data.routes[i].gpx_id + '</td>';
                            route_longest_query_table.innerHTML += str;
                        }
                        alert('Executed N Route Query');
                        console.log('Executed N Route Query');
                    } else {
                        console.log('Error executing N route query, ensure login');
                        alert('Error executing N route query, ensure login');
                    }
                },
                error: function (error) {
                    console.log(error);
                    alert("Error executing N route query, ensure login");
                }
            });
        } else {
            alert('Please select a file and input a positive integer N');
        }
    });
});

function updateFileLog() {
    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/getAllGPX',
        success: function (data) {
            if (!data.stat) {
                console.log('Error adding one of the files to log, may be invalid');
                alert('Error adding one of the files to log, may be invalid');
            }

            if (data.array.length > 0) {
                for (let i in data.array) {
                    console.log('Adding file to file log');
                    makeRowFileLog(data.array[i]);
                    alert('Uploaded ' + data.array[i].filename);
                }
            } else {
                file_log_table_body.innerHTML = "<tr><td>No Files</td></tr>";
            }
        },
        error: function (error) {
            console.log(error);
            alert("Error occurred updating file log");
        }
    });
}

function addToFileLog(filename) {
    jQuery.ajax({
        type: 'get',
        url: '/getSingleGPX',
        data: {
            name: filename
        },
        success: function (data) {
            if (!data.stat) {
                alert('Error adding file ' + filename + ' to log, may be invalid');
                console.log('Error adding file to log');
            } else {
                makeRowFileLog(data.son);
                if (lastDelta != -1) {
                    updatePathBetween(false);
                }
                alert('Uploaded ' + data.son.filename);
            }
        },
        error: function (error) {
            console.log(error);
            alert("Error occurred updating file log with " + filename);
        }
    });
}

function updatePathBetween(isAlert) {
    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/getPathBetween',
        data: {
            start: lastStartPoint,
            end: lastEndPoint,
            delta: lastDelta
        },
        success: function (data) {
            if (data.status) {
                path_between_table_body.innerHTML = "";
                for (let i in data.routeList) {
                    data.routeList[i]['component'] = "Route " + (+i + +1);
                    addComponentToPath(data.routeList[i]);
                }
                for (let i in data.trackList) {
                    data.trackList[i]['component'] = "Track " + (+i + +1);
                    addComponentToPath(data.trackList[i]);
                }
                if (isAlert) {
                    alert('Loaded path table with components');
                    console.log('Found path between');
                }
            } else {
                console.log('Error reading file');
                alert('Error getting path');
            }
        },
        error: function (error) {
            alert('Error occured making request');
            console.log(error);
        }
    });
}

function makeRowFileLog(son) {
    let str = "";
    files += 1;
    if (loggedIn) {
        document.getElementById('store-all').disabled = false;
    }
    str += '<tr><td><a href=\"' + son.filename + '\" download>' + son.filename + '</a></td>';
    str += '<td>' + son.version + '</td>' + '<td>' + son.creator + '</td>';
    str += '<td>' + son.numWaypoints + '</td>' + '<td>' + son.numRoutes + '</td>';
    str += '<td>' + son.numTracks + '</td>';
    str += '</tr>';
    if (file_log_table_body.innerHTML.includes("No Files")) {
        file_log_table_body.innerHTML = str;
    } else {
        file_log_table_body.innerHTML += str;
    }
    view_panel_drop_down.innerHTML += '<option>' + son.filename + '</option>';
    route_drop_down.innerHTML += '<option>' + son.filename + '</option>';
    
}

function addComponentToView(son) {
    let str = "";
    str += '<tr><td>' + son.component + '</td>' + '<td>' + son.name + '</td>' + '<td>' + son.numPoints + '</td>';
    str += '<td>' + son.len + 'm</td>' + '<td>' + son.loop.toString().toUpperCase() + '</td>';
    str += '</tr>';
    view_panel_table_body.innerHTML += str;
    rename_drop_down.innerHTML += '<option>' + son.component + '</option>';
    show_other_drop_down.innerHTML += '<option>' + son.component + '</option>';
}

function addComponentToPath(son) {
    let str = "";
    str += '<tr><td>' + son.component + '</td>' + '<td>' + son.name + '</td>' + '<td>' + son.numPoints + '</td>';
    str += '<td>' + son.len + 'm</td>' + '<td>' + son.loop.toString().toUpperCase() + '</td>';
    str += '</tr>';
    path_between_table_body.innerHTML += str;
}

function updateComponents() {
    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/showComponents',
        data: {
            filename: lastValidFileViewPanel
        },
        success: function (data) {
            if (data.status) {
                view_panel_table_body.innerHTML = "";
                rename_drop_down.innerHTML = "<option>Component to rename</option>";
                show_other_drop_down.innerHTML = "<option>Show Other Data of</option>";
                for (let i in data.routes) {
                    data.routes[i]['component'] = "Route " + (+i + +1);
                    addComponentToView(data.routes[i]);
                }
                for (let i in data.tracks) {
                    data.tracks[i]['component'] = "Track " + (+i + +1);
                    addComponentToView(data.tracks[i]);
                }
                console.log('Loaded components');
            } else {
                console.log('Error reading file');
                alert('Error reading ' + lastValidFileViewPanel);
            }
        },
        error: function (error) {
            alert('Error occured making request');
            console.log(error);
        }
    });
}

function updateFileLogRoute(filename) {
    // Find file in table and change number of waypoints and routes
    for (let i = 1; i < file_log_table_body.childNodes.length; i++) {
        if (filename === file_log_table_body.childNodes[i].firstChild.innerText) {
            file_log_table_body.childNodes[i].childNodes[4].innerHTML = parseInt(file_log_table_body.childNodes[i].childNodes[4].innerHTML) + +1;
            console.log('updated file log route');
            return;
        }
    }
    console.log('not updated file log route');
}

function rebuildTable(filename) {
    if (loggedIn) {
        jQuery.ajax({
            type: 'get',
            dataType: 'json',
            url: '/updateFile',
            data: {
                file: filename
            },
            success: function (data) {
                if (data.status) {
                    if (data.exists) {
                        getStatus();
                    }
                } else {
                    console.log('Error updating database, ensure login');
                    alert('Error updating database, ensure login');
                }
            },
            error: function(error) {
                console.log(error);
                alert("Error updating database, ensure login");
            }
        });
    }
}

function getStatus() {
    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/displayStatus',
        success: function (data) {
            if (data.status) {
                alert('Database has ' + data.files + ' files, ' + data.routes + ' routes, and ' + data.points + ' points');
                console.log('Displayed status');
                if (data.files != 0) {
                    document.getElementById('clear-all').disabled = false;
                }
            } else {
                console.log('Error Displaying data');
                alert('Error Displaying data, ensure login');
            }
        },
        error: function (error) {
            console.log(error);
            alert("Error occurred displaying data, ensure login");
        }
    });
}

function updateDBDropdowns() {
    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/getFilesDB',
        success: function(data) {
            if (data.status) {
                route_query_dropdown.innerHTML = '<option>All Files</option>';
                route_longest_query_dropdown.innerHTML = '<option>No file selected</option>';
                point_route_query_dropdown.innerHTML = '<option>No file selected</option>';
                point_file_query_dropdown.innerHTML = '<option>No file selected</option>';
                for (let i in data.files) {
                    route_query_dropdown.innerHTML += '<option>' + data.files[i].file_name + '</option>';
                    route_longest_query_dropdown.innerHTML += '<option>' + data.files[i].file_name + '</option>';
                    point_route_query_dropdown.innerHTML += '<option>' + data.files[i].file_name + '</option>';
                    point_file_query_dropdown.innerHTML += '<option>' + data.files[i].file_name + '</option>';
                }
                
            } else {
                console.log(error);
                alert("Error occurred updating query dropdowns, ensure login");
            }
        },
        error: function(data) {
            console.log(error);
            alert("Error occurred updating query dropdowns, ensure login");
        }
    });
}