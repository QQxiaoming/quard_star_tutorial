(:*******************************************************:)
(: Test: op-logical-or-111.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or' using unsignedLong values       :)
(:*******************************************************:)

   <return>
     { xs:unsignedLong(99999999999999999) or
          xs:unsignedLong(99999999999999999) }
   </return>
