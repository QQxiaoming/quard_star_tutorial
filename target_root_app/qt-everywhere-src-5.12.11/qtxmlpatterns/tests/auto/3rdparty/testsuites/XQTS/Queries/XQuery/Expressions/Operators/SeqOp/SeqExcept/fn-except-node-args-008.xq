(:*******************************************************:) 
(: Test: fn-except-node-args-008.xq          :) 
(: Written By: Ravindranath Chennoju                             :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg: text node & node                        :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare variable $input-context external; 
(: insert-end :) 

for $node in  
$input-context/bib/book/title/text() except $input-context/bib/book[3]/title/text() 
return <a> {$node} </a>
