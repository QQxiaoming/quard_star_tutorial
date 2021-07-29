(:*******************************************************:)
(:Test: fn-lang1args-3                                    :)
(:Written By: Carmelo Montanez                            :)
(:Date: Tue Apr 12 16:29:03 GMT-05:00 2005                :)
(:Purpose: Evaluates The "lang" function                 :)
(: with the arguments set as follows:                    :)
(:$testlang = "eN", node with no "xml:lang" attribute    :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:lang(xs:string("eN"),$input-context/root[1]/time[1])