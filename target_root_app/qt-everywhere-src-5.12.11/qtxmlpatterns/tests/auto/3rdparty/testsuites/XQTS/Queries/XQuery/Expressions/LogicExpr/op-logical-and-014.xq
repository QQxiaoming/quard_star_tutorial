(:*******************************************************:)
(: Test: op-logical-and-014.xq                           :)
(: Written By: Lalith Kumar                              :)
(: Date: Thu May 12 05:50:40 2005                        :)
(: Purpose: Logical 'and' using sequences                :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

   <return>
     { (0) and ($input-context/bib/book/price/text()) }
   </return>
