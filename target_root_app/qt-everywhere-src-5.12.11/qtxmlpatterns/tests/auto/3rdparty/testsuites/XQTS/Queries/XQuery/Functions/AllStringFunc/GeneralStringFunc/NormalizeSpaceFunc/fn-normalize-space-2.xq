(: Name: fn-normalize-space-2 :)
(: Description: Evaluation of fn-normalize-space function with no arguments and no context node. :)

declare namespace eg = "http://example.org";

declare function eg:noContextFunction()
 {
   fn:normalize-space()
};

eg:noContextFunction()