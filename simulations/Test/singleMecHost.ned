//
//                           Simu5G
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

package syntetictracegenerator.simulations.Test;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import simu5g.world.radio.LteChannelControl;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.Upf;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.NR.NRUe;
import simu5g.nodes.mec.MECHost;

import simu5g.nodes.mec.MECOrchestrator.MecOrchestrator;
import simu5g.nodes.mec.UALCMP.UALCMP;

import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;



network singleMecHost
{
    parameters:

        int numCars = default(0);

        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        @display("bgb=804.955,772.59503");

    submodules:

        routingRecorder: RoutingTableRecorder {
            @display("p=690,104.88;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=690,184.92");
        }
        veinsManager: VeinsInetManager {
            @display("p=50,227;is=s");
        }
        channelControl: LteChannelControl {
            @display("p=690,24.84;is=s");
        }
        binder: Binder {
            @display("p=690,264.96;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=691.38,342.24;is=s");
        }
        upf: Upf {
            @display("p=427.75876,390.3425;is=l");
        }
        gNodeB1: gNodeB {
            @display("p=141.575,64.72;is=vl");
        }
        gNodeB2: gNodeB {
            @display("p=141.575,677.5375;is=vl");
        }
        //# MEC module
        mecHost: MECHost {
            @display("p=437.664,251.424;is=l");
        }

        mecOrchestrator: MecOrchestrator {
            @display("p=566.3,390.3425;is=l");
        }

        ualcmp: UALCMP {
            @display("p=492.47876,389.33124;i=device/smallrouter");
        }


    connections allowunconnected:

        upf.pppg++ <--> Eth10G <--> gNodeB1.ppp;
        upf.pppg++ <--> Eth10G <--> gNodeB2.ppp;
        ualcmp.ppp++ <--> Eth10G <--> upf.filterGate;

        ualcmp.toMecOrchestrator --> mecOrchestrator.fromUALCMP;
        ualcmp.fromMecOrchestrator <-- mecOrchestrator.toUALCMP;

		//# X2 connections
        gNodeB1.x2++ <--> Eth10G <--> gNodeB2.x2++;

        //#	mec
        // direct connection between gNodeB1 and mecHost
        mecHost.pppENB++ <--> Eth10G <--> gNodeB1.pppMEHost;
}
