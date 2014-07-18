import QtQuick 2.2
import QtQuick.Controls 1.1

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: qsTr("License Agreement")

    property bool accepted: false

    Image {
        anchors.fill: parent
        source: "/images/background.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: parent.width / 15
        radius: anchors.margins / 2
        color: "#88FFFFFF"
        clip: true

        Flickable {
            anchors {
                fill: parent
                leftMargin: parent.radius
                rightMargin: anchors.leftMargin
            }
            contentHeight: licenseArea.height
            contentWidth: width
            maximumFlickVelocity: 3000

            Rectangle {
                id: licenseArea
                color: "transparent"
                height: childrenRect.height
                width: parent.width

                Text {
                    id: licenseText
                    textFormat: TextEdit.RichText
                    font.family: "helvetica"
                    font.pointSize: 14
                    font.wordSpacing: 5
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: "<center><h1>Welcome to " + Qt.application.name + "</h1></center><br/><br/>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras eu nibh consectetur lacus convallis tincidunt. Cras iaculis leo est, non interdum sapien viverra sed. Integer sodales odio lectus, a pharetra risus pellentesque at. Aliquam lobortis est sed ante convallis aliquam vel id dolor. Praesent semper mauris eros, a consequat metus aliquet a. Donec vitae nisi fringilla nulla ullamcorper tincidunt et nec quam. Nullam porta purus tincidunt tortor cursus commodo. In sit amet viverra diam. Sed in elit fermentum, pretium tortor vitae, fringilla quam. Nullam id venenatis felis. Integer tristique feugiat luctus. Vestibulum vel lacus diam. In non elit ut eros tincidunt dapibus.
<p/>
Aliquam sit amet orci sapien. Donec facilisis odio id nisl dapibus, vel convallis ipsum semper. Aenean dapibus accumsan ligula sed pulvinar. Vivamus nec erat arcu. Proin in bibendum tortor, at bibendum quam. Aliquam ut tempor mauris, sed tristique tortor. In volutpat orci nec nibh accumsan, ac ultricies lacus ullamcorper. Etiam nec neque sit amet orci tincidunt sollicitudin. Proin quis porttitor lorem, sit amet commodo tortor. Cras nec tortor dapibus, consectetur tortor nec, volutpat purus. Curabitur ligula nibh, sodales nec diam non, luctus ornare lorem. Nam tristique, augue quis egestas dapibus, libero nulla fringilla nulla, nec pretium massa nulla eget nulla. Sed posuere metus ut eros porta euismod.
<p/>
Proin rhoncus, mauris vitae sodales interdum, libero sem euismod nisl, a vestibulum magna mauris sit amet est. In ut sem quis risus consequat placerat quis id est. In convallis risus mi, quis lacinia nisl eleifend non. Sed eu ligula ultricies, condimentum justo et, euismod urna. Sed eu dui porta, ultricies ipsum sed, euismod leo. Nulla ut aliquet quam, in dignissim nisl. Vestibulum vulputate risus non sodales tempor. Sed ut mattis eros. Nullam neque eros, auctor non ullamcorper eu, scelerisque ac sapien.
<p/>
Vivamus id odio tempus, faucibus lorem sed, malesuada tellus. Curabitur non nisl sollicitudin, gravida dui ac, mattis orci. In elit eros, tincidunt eget feugiat eu, eleifend et enim. Donec consequat quis leo at vulputate. Nam condimentum mi auctor purus volutpat, sit amet commodo lectus porttitor. Morbi vulputate ultricies orci at luctus. Phasellus cursus aliquam accumsan. Cras quis eros at augue accumsan fringilla in tempus quam. Quisque at lectus sed urna semper suscipit. Etiam accumsan facilisis ultricies. Quisque vitae imperdiet est, et tincidunt diam. Mauris vulputate orci quis turpis mollis, non scelerisque velit tempor. Cras cursus vehicula volutpat. Maecenas lacinia pellentesque odio, sed euismod tortor convallis nec. Pellentesque massa augue, pulvinar vitae dignissim nec, convallis et eros.
<p/>
Pellentesque nec porta mauris. Vestibulum sit amet tortor euismod, laoreet velit in, elementum massa. Donec tincidunt leo id est laoreet porta. In condimentum viverra dapibus. Proin non ante luctus, auctor dui a, pretium ligula. Pellentesque blandit orci neque, ac congue eros consequat at. In hac habitasse platea dictumst. Suspendisse auctor pulvinar turpis ac pulvinar.
<p/>
Integer nec accumsan elit. Mauris rutrum arcu eget elit fermentum pharetra at a elit. Aliquam erat volutpat. Fusce id arcu odio. Nulla iaculis elit nisi, eu tempor felis imperdiet sit amet. Praesent varius nibh vitae elementum dapibus. Donec vehicula enim erat, eu interdum orci scelerisque a. In id porttitor ante, in convallis nisl. Vestibulum suscipit lorem nulla. Duis ornare rhoncus diam eget cursus. Proin consectetur libero vitae augue rutrum, et posuere felis commodo. Maecenas in leo in risus lobortis tincidunt. Suspendisse facilisis massa ac turpis bibendum, eu porta turpis viverra.
<p/>
Sed placerat mauris ut quam ultrices, ac imperdiet velit feugiat. Pellentesque tincidunt felis in sem adipiscing rutrum. Proin in molestie sem. Quisque fermentum condimentum dolor, faucibus molestie erat tincidunt et. Sed iaculis purus nec mi facilisis ullamcorper. Proin facilisis purus nec ipsum hendrerit, id euismod neque blandit. Mauris sapien est, consectetur in pellentesque sed, condimentum non ante. Praesent scelerisque felis vel dolor adipiscing rhoncus. Quisque congue odio vel nisi eleifend, in luctus lacus lacinia. Quisque pellentesque sapien sit amet ligula adipiscing convallis. Donec lacinia odio erat, at rhoncus ligula vehicula id. Sed nec nibh eu mi convallis interdum at sed arcu. Donec at leo sit amet arcu hendrerit bibendum eget eget arcu. Nam mauris augue, euismod sit amet erat et, pellentesque commodo odio.
<p/>
In id ante odio. Praesent sit amet lectus eu ipsum pretium mattis tempor et risus. Morbi hendrerit nisl sit amet pretium sodales. Curabitur tristique bibendum imperdiet. Nulla vestibulum posuere adipiscing. Fusce eu quam tortor. Curabitur at commodo dui, vel adipiscing dolor. Sed ut venenatis velit, sit amet pretium risus. Morbi sed mi venenatis, consequat ligula et, molestie lacus.
<p/>
Ut nec massa eu nunc lobortis volutpat consectetur et metus. Nulla sit amet ultricies tellus. Nullam sollicitudin blandit consectetur. Nam ante purus, commodo id odio eu, lacinia laoreet est. Praesent sodales lorem pharetra luctus aliquam. In tristique, dolor eget aliquet elementum, nisl eros lobortis urna, eget vulputate urna mi sed velit. In ultrices rhoncus mattis. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nunc sed elementum metus. Nulla a augue at turpis pretium elementum quis eu arcu.
<p/>
Nulla facilisi. Proin vel bibendum purus. Etiam in viverra lectus. Duis semper dignissim sapien sit amet posuere. Sed euismod est id semper mattis. Nulla vel ornare urna. Duis eu lectus iaculis, vulputate sem vulputate, bibendum enim. Donec et elit tincidunt, volutpat sem quis, lacinia mauris. Cras dignissim sodales ipsum quis vestibulum. Nulla facilisi.
<p/>
Cras commodo felis in nunc bibendum, tristique lobortis lectus gravida. Proin id mollis orci. Mauris in tellus dictum, pretium elit vitae, vulputate dui. Nam eget aliquam nunc. Duis eget orci sit amet nisl pellentesque ultricies vitae at massa. Proin quis libero ut nunc gravida venenatis vel sed lacus. Donec consequat dictum pulvinar. Donec malesuada ut sapien at suscipit. Aenean porttitor arcu eu condimentum dictum. Curabitur a tempor mauris. Morbi nisi tellus, ornare tempor rutrum eget, ornare vel ligula. Aliquam sem mi, iaculis non rhoncus at, tempus porta neque. Praesent sollicitudin arcu sodales enim lacinia posuere. Quisque a velit eget arcu ornare venenatis. Pellentesque iaculis vitae lorem eget suscipit. Ut tincidunt molestie porttitor.
<p/>
Vestibulum vitae velit vitae leo rhoncus dapibus ut tincidunt sem. Etiam pulvinar, nunc at placerat lacinia, quam neque rutrum quam, eu ultrices nibh libero varius lorem. Praesent porttitor elit sit amet justo suscipit, id tincidunt lectus tempor. Aliquam ut erat vulputate, tristique quam quis, condimentum odio. Pellentesque lobortis blandit purus. Fusce vel congue lectus. Aliquam vel tortor in tellus sagittis vehicula. Donec volutpat ante in facilisis tristique. Nunc ultrices est in ligula imperdiet placerat. Interdum et malesuada fames ac ante ipsum primis in faucibus. Donec euismod massa metus, sit amet faucibus nulla blandit pharetra. Aliquam convallis arcu at nulla fermentum posuere. In hac habitasse platea dictumst.
<p/>
In adipiscing in nisi quis aliquam. Etiam tellus eros, sagittis vitae dolor a, convallis sodales risus. Donec vel velit vitae nisi tempor imperdiet in quis mi. Aliquam velit justo, hendrerit sit amet molestie et, porttitor at libero. Donec sollicitudin consequat massa eget ultricies. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aliquam imperdiet accumsan elementum. Sed tristique ipsum at enim vestibulum consectetur. Vestibulum ullamcorper consectetur mauris, tristique eleifend mi sagittis at. Sed lorem nisl, molestie et ornare ac, ornare nec nulla.
<p/>
Fusce lacus felis, posuere in ligula ac, porttitor faucibus sapien. Aenean suscipit eleifend pharetra. Phasellus libero diam, malesuada nec auctor et, cursus id erat. In vel gravida ipsum. Pellentesque et placerat leo. Nunc pulvinar luctus dui. Aenean sed ipsum at sem fringilla cursus. Phasellus at aliquam leo. Cras aliquam dolor quis nisi semper lacinia.
<p/>
Integer eros mi, scelerisque egestas egestas eu, dapibus in erat. Vestibulum molestie leo non magna mollis, id facilisis mauris vestibulum. Pellentesque volutpat mollis felis in vulputate. Vivamus ut diam ante. Pellentesque ac commodo diam. Donec non magna nunc. Mauris dignissim quam quis commodo commodo. Donec facilisis, mi eu consequat lobortis, massa enim molestie augue, ac dignissim est quam nec justo. Curabitur molestie, odio et volutpat ultricies, erat arcu ornare felis, nec adipiscing augue purus vehicula dui. Vivamus fermentum quam est, sed vestibulum elit commodo sed. Vivamus imperdiet eget erat eu fermentum. Curabitur interdum vehicula magna quis porta.
<p/>
Fusce a fermentum tellus. Donec euismod ipsum sit amet arcu laoreet, quis consectetur metus sollicitudin. Pellentesque ultrices commodo massa sit amet sagittis. Fusce et aliquet lectus, a ultricies sem. Ut in ultricies nibh. Curabitur sollicitudin volutpat quam, in malesuada orci dapibus et. Quisque congue sapien risus, sit amet laoreet dui vestibulum nec. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos.
<p/>
Nullam nec urna lobortis, placerat tellus sed, mattis eros. Vestibulum vitae magna neque. Duis quis magna sed enim imperdiet consequat at vitae diam. Curabitur nec diam quam. Etiam vel accumsan nunc. Proin fringilla vel dui in ullamcorper. In sed sem dolor. Nulla tellus tellus, blandit in est sed, tempus dapibus diam. Mauris id aliquet ipsum. Morbi eget egestas ante.
<p/>
Vivamus interdum est vitae dignissim interdum. Phasellus hendrerit non eros a ullamcorper. Integer molestie quam a interdum consectetur. Mauris ut vestibulum sem, vel lacinia urna. Nulla nibh purus, adipiscing condimentum elit vel, laoreet placerat diam. Cras non velit non purus varius ornare. Etiam tortor leo, convallis et sem id, viverra dignissim metus. Nulla dapibus congue auctor. Aenean vel tincidunt urna.
<p/>
Nullam bibendum, purus quis pulvinar tempus, est metus lobortis augue, eu blandit tortor enim sed magna. Curabitur nec accumsan magna. Sed varius quis justo vel consequat. Etiam sodales lacus id nunc rutrum laoreet a in nibh. Aliquam erat volutpat. Curabitur posuere velit quis mauris hendrerit mattis. Maecenas lacinia dui nisi, a sollicitudin quam egestas eu. Phasellus nunc risus, congue eu adipiscing at, mattis id dui. Suspendisse faucibus leo et ante commodo dapibus. Nunc volutpat quam at massa sodales adipiscing. Donec eu sem placerat, molestie leo vitae, vulputate quam. Morbi a semper leo. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean laoreet orci at tempus convallis.
<p/>
Duis in volutpat leo. Vivamus sed augue eros. Suspendisse nec justo eget eros elementum dapibus. Aliquam at ultricies ante, nec pulvinar turpis. Phasellus vehicula, lacus et ornare congue, mi felis volutpat mi, fringilla blandit est sem nec orci. Suspendisse quis sapien at elit pretium tristique quis at augue. Curabitur sed lectus mauris. Quisque non facilisis justo, a facilisis enim. Aenean metus diam, posuere nec auctor sed, blandit vel diam. Duis libero metus, facilisis id elit ac, consectetur lobortis augue. Nam sit amet dapibus dolor. Interdum et malesuada fames ac ante ipsum primis in faucibus. Curabitur tristique dignissim ligula non cursus. Aliquam suscipit, nisi sed scelerisque iaculis, nulla urna tristique ipsum, ac rutrum quam nulla a dolor."
                }

                Row {
                    anchors.top: licenseText.bottom
                    anchors.topMargin: 30
                    anchors.horizontalCenter: parent.horizontalCenter
                    Button {
                        text: qsTr("Reject")
                        onClicked: {
                            accepted = false
                            Qt.quit()
                        }
                    }
                    Button {
                        text: qsTr("Accept")
                        onClicked: {
                            accepted = true
                            Qt.quit()
                        }
                    }
                }
            }
        }
    }
}
