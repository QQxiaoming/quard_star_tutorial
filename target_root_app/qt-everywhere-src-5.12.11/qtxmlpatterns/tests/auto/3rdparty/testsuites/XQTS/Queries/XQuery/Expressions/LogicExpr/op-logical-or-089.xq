(:*******************************************************:)
(: Test: op-logical-or-089.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using nonNegativeInteger values:)
(:*******************************************************:)

   <return>
     { xs:nonNegativeInteger(99999999999999999)
         or xs:nonNegativeInteger(99999999999999999) }
   </return>
