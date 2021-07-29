(:*******************************************************:) 
(: Test: fn-intersect-node-args-002.xq          :) 
(: Written By: Ravindranath Chennoju                             :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg: incorrect nodes                         :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare variable $input-context external;
(: insert-end :) 
 
$input-context/bib/book/title intersect $input-context/bib/book[1]/title
