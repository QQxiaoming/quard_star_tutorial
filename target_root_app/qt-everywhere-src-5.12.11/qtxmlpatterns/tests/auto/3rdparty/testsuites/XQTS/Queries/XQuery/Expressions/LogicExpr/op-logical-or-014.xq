(:*******************************************************:)
(: Test: op-logical-or-014.xq                            :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:53:51 2005                        :)
(: Purpose: Logical 'or'  using sequences values         :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

   <return>
     { (0) or ($input-context/bib/book/price/text()) }
   </return>
