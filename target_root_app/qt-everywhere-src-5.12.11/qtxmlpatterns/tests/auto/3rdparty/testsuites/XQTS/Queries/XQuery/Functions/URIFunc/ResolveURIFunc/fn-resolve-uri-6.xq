(: Name: fn-resolve-uri-6 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI (Two argument version of function).:)
(: Use the fn-string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri("examples","http://www.examples.com/"))