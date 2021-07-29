(:*******************************************************:)
(: Test: op-logical-and-111.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using unsignedLong values      :)
(:*******************************************************:)

   <return>
     { xs:unsignedLong(99999999999999999) and
          xs:unsignedLong(99999999999999999) }
   </return>
