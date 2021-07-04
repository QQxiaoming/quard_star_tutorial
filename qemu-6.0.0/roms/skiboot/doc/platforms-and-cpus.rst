Supported platforms & CPUs
==========================

NB. This file should only contain publicly available information.

CPUs
----

=============== =============== =====================
Name            PVR             Other names
=============== =============== =====================
Power8E         0x004bxxxx      Murano
Power8          0x004dxxxx      Venice
Power8NVL       0x004cxxxx      Naples
Power9N         0x004e0xxx      Nimbus 12 small core
Power9N         0x004e1xxx      Nimbus 24 small core
Power9C         0x004e2xxx      Cumulus 12 small core
Power9C         0x004e3xxx      Cumulus 24 small core
Power9P         0x004fxxxx      Axone
=============== =============== =====================

Platforms
---------

======== ============ =========== ================== ========================== ============================= =======
Platform Sub platform Host CPU(s) Manufacturer       compatible                 Other names/Notes             Link(s)
======== ============ =========== ================== ========================== ============================= =======
astbmc   barreleye    Power8      Ingrasys (Foxconn) "ingrasys,barreleye"       Barreleye, Rackspace machine
astbmc   firestone    Power8      Wistron, IBM       "ibm,firestone"            Firestone, S822LC             [#]_
astbmc   garrison     Power8      IBM                "ibm,garrison"             Minsky, "S822LC for HPC"      [#]_
astbmc   habanero     Power8      Tyan               "tyan,habanero"            Habanero, TN71-BP012
astbmc   p8dtu        Power8      Supermicro         "supermicro,p8dtu1u"       Briggs, "S822LC for Big Data" [#]_ [#]_
astbmc   p8dtu        Power8      Supermicro         "supermicro,p8dtu2u"       Stratton, S821LC              [#]_ [#]_
astbmc   p8dnu        Power8      Supermicro         "supermicro,p8dnu(1u|2u)?"
astbmc   p9sdu        Power9      Supermicro         "supermicro,p9dsu"         Boston, LC921/LC922           [#]_
astbmc   palmetto     Power8      Tyan               "tyan,palmetto"            Palmetto, GN70-BP010
astbmc   romulus      Power9                         "ibm,romulus"              Romulus
astbmc   vesnin       Power9      Yadro              "YADRO,vesnin"             VESNIN
astbmc   witherspoon  Power9                         "ibm,witherspoon"          Witherspoon, Newell, AC922    [#]_
astbmc   zaius        Power9      Ingrasys (Foxconn) "ingrasys,zaius"           Zaius, Google Machine
ibm-fsp  apollo       Power8                         "ibm,apollo"
ibm-fsp  firenze      Power8E     IBM                "ibm,firenze"              Tuleta, S812L, S822L          [#]_
ibm-fsp  zz           Power9                         "ibm,zz-(1|2)s(2|4)u"
rhesus   n/a          Power8E                        "ibm,powernv"              Rhesus, Google machine
======== ============ =========== ================== ========================== ============================= =======


.. rubric:: Footnotes

.. Firestone
.. [#] `IBM Back In HPC With Power Systems LC Clusters <https://www.nextplatform.com/2015/10/08/ibm-back-in-hpc-with-power-systems-lc-clusters/>`_
.. Minsky
.. [#] `Refreshed IBM Power Linux Systems Add NVLink <https://www.nextplatform.com/2016/09/08/refreshed-ibm-power-linux-systems-add-nvlink/>`_
.. Briggs
.. [#] `Refreshed IBM Power Linux Systems Add NVLink <https://www.nextplatform.com/2016/09/08/refreshed-ibm-power-linux-systems-add-nvlink/>`_
.. [#] `First Look IBM POWER8 S822LC 8001-22C (Briggs) <https://www.youtube.com/watch?v=TnW-NcLR28g>`_
.. Stratton
.. [#] `Refreshed IBM Power Linux Systems Add NVLink <https://www.nextplatform.com/2016/09/08/refreshed-ibm-power-linux-systems-add-nvlink/>`_
.. [#] `First Look IBM POWER8 S821LC 8001-12C (Stratton) <https://www.youtube.com/watch?v=OM3wU4Uu8LI>`_
.. [#] `Boston Power9s Set To Debut <https://www.itjungle.com/2018/05/14/boston-power9s-set-to-debut/>`_
.. [#] `POWER9 TO THE PEOPLE <https://www.nextplatform.com/2017/12/05/power9-to-the-people/>`_
.. Tuleta
.. [#] `IBM Power System S812L and IBM Power System S822L <https://www.ibm.com/au-en/marketplace/power-system-s812l-s822l>`_
