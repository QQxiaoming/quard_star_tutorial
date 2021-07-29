(: Name: context-item-1 :)
(: Description: Evaluation of contect item expression for which the context item is not defined. :)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:boolean(.)
};

eg:noContextFunction()