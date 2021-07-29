(:*******************************************************:)
(: Test: op-logical-or-120.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or' using int values                :)
(:*******************************************************:)

   <return>
     { xs:int(2147483647) or xs:int(-2147483648) }
   </return>
