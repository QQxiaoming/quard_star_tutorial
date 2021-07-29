(: Name: optiondeclprolog-1 :)
(: Description: Evaluation of a simple prolog option declaration.:)

declare namespace exq = "http://example.org/XQueryImplementation";
declare option exq:java-class "math = java.lang.Math";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"