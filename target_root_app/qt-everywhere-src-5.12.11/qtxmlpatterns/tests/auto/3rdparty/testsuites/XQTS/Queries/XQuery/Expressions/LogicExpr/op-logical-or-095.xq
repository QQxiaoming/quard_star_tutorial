(:*******************************************************:)
(: Test: op-logical-or-095.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using positiveInteger values   :)
(:*******************************************************:)

   <return>
     { xs:positiveInteger(99999999999999999)
         or xs:positiveInteger(99999999999999999) }
   </return>
