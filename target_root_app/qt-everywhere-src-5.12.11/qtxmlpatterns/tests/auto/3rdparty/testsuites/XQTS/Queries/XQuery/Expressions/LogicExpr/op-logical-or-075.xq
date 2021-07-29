(:*******************************************************:)
(: Test: op-logical-or-075.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using integer values           :)
(:*******************************************************:)

   <return>
     { xs:integer(99999999999999999)
         or xs:integer(-99999999999999999) }
   </return>
