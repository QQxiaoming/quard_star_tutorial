(: Name: fn-resolve-uri-11 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI and given as a an argument to fn:substring function (Two argument version of function).:)
(: Use the fn-string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri("examples",fn:substring("1234http://www.examples.com/",5)))