(: Name: fn-string-length-18 :)
(: Description: Evaluation of string-length function with no argument and no context item defined. :)

declare namespace eg = "http://example.org";

declare function eg:noContextFunction()
 {
   fn:string-length()
};

eg:noContextFunction()