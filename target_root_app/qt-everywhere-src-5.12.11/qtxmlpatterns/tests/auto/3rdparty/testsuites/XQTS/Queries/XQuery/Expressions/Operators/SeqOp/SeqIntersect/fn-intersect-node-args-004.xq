(:*******************************************************:) 
(: Test: fn-intersect-node-args-004.xq          :) 
(: Written By: Ravindranath Chennoju                             :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg: text node & node                        :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare variable $input-context external;
(: insert-end :) 
 
$input-context/bib/book[3]/title/text() intersect $input-context/bib/book/title/text() 
