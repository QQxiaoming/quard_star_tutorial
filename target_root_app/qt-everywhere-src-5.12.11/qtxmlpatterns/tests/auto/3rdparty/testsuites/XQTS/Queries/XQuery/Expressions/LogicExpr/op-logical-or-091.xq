(:*******************************************************:)
(: Test: op-logical-or-091.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using negativeInteger values   :)
(:*******************************************************:)

   <return>
     { xs:negativeInteger(-99999999999999999)
         or xs:negativeInteger(-1) }
   </return>
