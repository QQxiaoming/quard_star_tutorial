(:*******************************************************:)
(: Test: modules-none.xq                                 :)
(: Written By: Carmelo Montanez                          :)
(: Date: 2006/01/19                                      :)
(: Purpose: Test for importing non-existing library module.:)
(:*******************************************************:)

(: insert-start :)
import module namespace none="http://www.w3.org/TestModules/none" at "empty-lib.xq";
declare variable $input-context external;
(: insert-end :)

"abc"
