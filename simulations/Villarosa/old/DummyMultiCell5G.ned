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


network DummyMultiCell5G
{
    parameters:
        int numGNB = default(10);

        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        @display("bgb=950.225,608.1937");

    submodules:
        channelControl: LteChannelControl {
            @display("p=50,25;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=50,75;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=49.75,156.7125");
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
            @display("p=50.184,444.31198");
        }
        //        server: StandardHost {
        //            @display("p=707.472,324.36;is=n;i=device/server");
        //        }
        router: Router {
            @display("p=499.392,192.168;i=device/smallrouter");
        }
        upf: Upf {
            @display("p=373.32,193.392;is=l");
        }
        gNBs[numGNB]: gNodeB {
            @display("p=214.2,193.392;is=vl");
        }

        mecHost: MECHost {
            @display("p=498.168,334.152;is=l");
        }
        mecOrchestrator: MecOrchestrator {
            @display("p=675.648,94.248;is=l");
        }
        ualcmp: UALCMP {
            @display("p=499.392,93.024;i=device/smallrouter");
        }

    connections:
        //        server.pppg++ <--> Eth10G <--> router.pppg++;  
        for i=0..numGNB-1 {
            upf.pppg++ <--> Eth10G {  length = 1000m; } <--> gNBs[i].ppp;
        }
        //for i=0..numGNB-2 {
        //    gNBs[i].x2++ <--> Eth10G <--> gNBs[i+1].x2++;
        //}

        router.pppg++ <--> Eth10G <--> upf.filterGate;
        router.pppg++ <--> Eth10G <--> ualcmp.ppp++;
        ualcmp.toMecOrchestrator --> mecOrchestrator.fromUALCMP;
        ualcmp.fromMecOrchestrator <-- mecOrchestrator.toUALCMP;
        mecHost.ppp++ <--> Eth10G <--> router.pppg++;


}
