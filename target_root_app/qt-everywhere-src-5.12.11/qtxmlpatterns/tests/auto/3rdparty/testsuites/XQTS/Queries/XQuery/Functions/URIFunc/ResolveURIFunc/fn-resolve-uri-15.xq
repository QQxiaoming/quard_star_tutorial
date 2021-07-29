(: Name: fn-resolve-uri-15 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI and given as a an argument to fn:substring-after function (Two argument version of function).:)
(: Use the fn:substring-after function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri("examples",fn:substring-after("123http://www.example.com/","123")))