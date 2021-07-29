(:*******************************************************:) 
(: Test: fn-indexof-mix-args-019.xq          :) 
(: Written By: Ravindranath Chennoju                     :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: Use external variable both in sequence and search parameter :) 
(:***********************************************************************:) 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)
 
 fn:index-of(($input-context/bib/book/publisher), $input-context/bib/book[1]/publisher[1]/text() cast as xs:string)
