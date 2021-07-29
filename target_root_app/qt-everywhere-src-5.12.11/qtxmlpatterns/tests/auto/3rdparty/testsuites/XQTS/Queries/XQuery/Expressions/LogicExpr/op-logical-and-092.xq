(:*******************************************************:)
(: Test: op-logical-and-092.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using negativeInteger values   :)
(:*******************************************************:)

   <return>
     { xs:negativeInteger(-99999999999999999)
         and xs:negativeInteger(-99999999999999999) }
   </return>
