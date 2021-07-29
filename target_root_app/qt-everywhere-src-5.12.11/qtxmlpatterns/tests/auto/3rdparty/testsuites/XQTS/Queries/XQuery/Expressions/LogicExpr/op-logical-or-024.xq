(:*******************************************************:)
(: Test: op-logical-or-024.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using string values            :)
(:*******************************************************:)

   <return>
     { xs:string("0") or xs:string('abc') }
   </return>
