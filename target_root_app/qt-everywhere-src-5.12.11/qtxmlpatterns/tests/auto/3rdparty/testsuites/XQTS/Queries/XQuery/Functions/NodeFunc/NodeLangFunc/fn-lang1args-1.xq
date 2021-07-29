(:*******************************************************:)
(:Test: fn-lang1args-1                                    :)
(:Written By: Carmelo Montanez                            :)
(:Date: Tue Apr 12 16:29:03 GMT-05:00 2005                :)
(:Purpose: Evaluates The "lang" function                 :)
(: with the arguments set as follows:                    :)
(:$testlang = "en", node with no "xml:lang" attribute    :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:lang(xs:string("en"),$input-context/root[1]/time[1])