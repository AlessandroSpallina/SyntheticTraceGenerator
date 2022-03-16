//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package syntetictracegenerator.simulations.Villarosa.old;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.ExtCell;
import simu5g.nodes.PgwStandard;
import simu5g.world.radio.LteChannelControl;
import simu5g.nodes.Upf;
import simu5g.nodes.mec.MECHost;
import simu5g.nodes.mec.MECOrchestrator.MecOrchestrator;
import simu5g.nodes.mec.UALCMP.UALCMP;
import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;
import inet.visualizer*.common.IntegratedVisualizer;
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;


network ExtraDummyMultiCell5G
{
    parameters:
        // int numGNB = default(10);

        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

        //@display("bgb=985.32,990.216;bgi=background/basicgrid,s");

    submodules:
        channelControl: LteChannelControl {
            @display("p=50,25;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=50,75;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=49.75,156.7125");
            config = xmldoc("demo.xml");
        }
        binder: Binder {
            @display("p=49.75,225.11874;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=50.993748,284.81876;is=s");
        }
        veinsManager: VeinsInetManager {
            @display("p=50.184,354.96;is=s");
        }
        visualizer: IntegratedVisualizer {
            @display("p=64,320");
        }
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=64,416");
        }
        //        server: StandardHost {
        //            @display("p=707.472,324.36;is=n;i=device/server");
        //        }
        router: Router {
            @display("p=687.888,222.76799;i=device/smallrouter");
        }
        upf: Upf {
            @display("p=539.784,203.18399;is=l");
        }
        gNodeB1: gNodeB {
            @display("p=214.2,85.68;is=vl");
        }
        gNodeB2: gNodeB {
            @display("p=434.52,96.696;is=vl");
        }
        //gNodeB3: gNodeB {
        //    @display("p=214.2,319.464;is=vl");
        //}
        //gNodeB4: gNodeB {
        //    @display("p=434.52,319.464;is=vl");
        //}
        mecHost: MECHost {
            @display("p=707.472,306;is=l");
        }
        mecOrchestrator: MecOrchestrator {
            @display("p=687.888,74.664;is=l");
        }
        ualcmp: UALCMP {
            @display("p=687.888,161.568;i=device/smallrouter");
        }

    connections:
        //        server.pppg++ <--> Eth10G <--> router.pppg++;  
        //for i=0..numGNB-1 {
        //    upf.pppg++ <--> Eth10G {  length = 1000m; } <--> gNBs[i].ppp;
        //}
        //for i=0..numGNB-2 {
        //    gNBs[i].x2++ <--> Eth10G <--> gNBs[i+1].x2++;
        //}

        upf.pppg++ <--> Eth10G {  length = 1000m; } <--> gNodeB1.ppp;
        upf.pppg++ <--> Eth10G {  length = 1000m; } <--> gNodeB2.ppp;
        //upf.pppg++ <--> Eth10G {  length = 1000m; } <--> gNodeB3.ppp;
        //upf.pppg++ <--> Eth10G {  length = 1000m; } <--> gNodeB4.ppp;

        gNodeB1.x2++ <--> Eth10G <--> gNodeB2.x2++;
        //gNodeB2.x2++ <--> Eth10G <--> gNodeB4.x2++;
        //gNodeB4.x2++ <--> Eth10G <--> gNodeB3.x2++;
        //gNodeB3.x2++ <--> Eth10G <--> gNodeB1.x2++;

        router.pppg++ <--> Eth10G <--> upf.filterGate;
        router.pppg++ <--> Eth10G <--> ualcmp.ppp++;
        ualcmp.toMecOrchestrator --> mecOrchestrator.fromUALCMP;
        ualcmp.fromMecOrchestrator <-- mecOrchestrator.toUALCMP;
        mecHost.ppp++ <--> Eth10G <--> router.pppg++;


}
