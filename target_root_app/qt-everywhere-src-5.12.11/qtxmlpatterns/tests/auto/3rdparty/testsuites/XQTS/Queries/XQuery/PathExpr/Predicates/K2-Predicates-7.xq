(:*******************************************************:)
(: Test: K2-Predicates-7                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply last() to ancestor-or-self within a predicate. :)
(:*******************************************************:)
<r>{<e xml:lang="ene"/>/(ancestor-or-self::*/@xml:lang)[last()]}</r>