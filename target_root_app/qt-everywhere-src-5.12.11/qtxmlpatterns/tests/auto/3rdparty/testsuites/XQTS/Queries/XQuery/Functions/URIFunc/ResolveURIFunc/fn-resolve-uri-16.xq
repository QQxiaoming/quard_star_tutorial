(: Name: fn-resolve-uri-16 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI and given as an argument to fn:string function (two argument version of function).:)
(: Use the fn-string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri(fn:string("http://www.examples.com/"),""))