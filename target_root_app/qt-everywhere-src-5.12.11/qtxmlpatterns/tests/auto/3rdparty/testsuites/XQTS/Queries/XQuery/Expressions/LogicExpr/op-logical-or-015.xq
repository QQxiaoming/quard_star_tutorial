(:*******************************************************:)
(: Test: op-logical-or-015.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using sequences values         :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

   <return>
     { ($input-context/bib/book/price/text()) or (1) }
   </return>
