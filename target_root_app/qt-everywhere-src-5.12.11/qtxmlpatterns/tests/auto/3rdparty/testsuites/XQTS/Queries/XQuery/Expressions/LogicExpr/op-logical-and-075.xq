(:*******************************************************:)
(: Test: op-logical-and-075.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using integer values           :)
(:*******************************************************:)

   <return>
     { xs:integer(99999999999999999)
         and xs:integer(-99999999999999999) }
   </return>
