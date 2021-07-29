(:*******************************************************:) 
(: Test: fn-indexof-mix-args-018.xq          :) 
(: Written By: Ravindranath Chennoju                             :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: Use external variable for the sequence parameter :) 
(:***********************************************************************:) 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)
 
 fn:index-of(($input-context/bib/book/publisher), "Addison-Wesley")
