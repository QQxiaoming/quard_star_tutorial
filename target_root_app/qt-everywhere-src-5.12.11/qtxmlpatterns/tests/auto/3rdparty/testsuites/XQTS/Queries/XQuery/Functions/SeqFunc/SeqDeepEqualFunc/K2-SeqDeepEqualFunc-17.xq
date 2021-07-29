(:*******************************************************:)
(: Test: K2-SeqDeepEqualFunc-17                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure processing instructions are ignored if children of a document node(#2). :)
(:*******************************************************:)
declare variable $d1 := document {()};
declare variable $d2 := document {<?target data?>};
deep-equal($d1, $d2)