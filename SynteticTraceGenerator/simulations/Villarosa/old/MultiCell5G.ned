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


network MultiCell5G
{
    parameters:
        int numGNB = default(10);
        // string imagePath = default("background/budapest");
        // @display("i=block/network2;bgi=background/villarosa;bgb=1798.4625,942.7625");

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
        server: StandardHost {
            @display("p=719.712,178.704;is=n;i=device/server");
        }
        router: Router {
            @display("p=570.384,177.48;i=device/smallrouter");
        }
        upf: Upf {
            @display("p=501.84,298.656;is=l");
        }
        gNB[numGNB]: gNodeB {
            @display("p=283.968,298.656;is=vl");
        }
        mecHost: MECHost {
            @display("p=403.91998,156.672;is=l");
        }
        mecOrchestrator: MecOrchestrator {
            @display("p=713.592,443.08798;is=l");
        }
        ualcmp: UALCMP {
            @display("p=665.856,325.58398;i=device/smallrouter");
        }

    connections:
//        server.pppg++ <--> Eth10G <--> router.pppg++;
//        router.pppg++ <--> Eth10G <--> upf.filterGate;
        upf.pppg++ <--> Eth10G <--> gNB[numGNB-1].ppp;
        ualcmp.ppp++ <--> Eth10G <--> upf.filterGate;
        ualcmp.toMecOrchestrator --> mecOrchestrator.fromUALCMP;
        ualcmp.fromMecOrchestrator <-- mecOrchestrator.toUALCMP;
        mecHost.pppENB++ <--> Eth10G <--> gNB[numGNB-1].pppMEHost;
}
