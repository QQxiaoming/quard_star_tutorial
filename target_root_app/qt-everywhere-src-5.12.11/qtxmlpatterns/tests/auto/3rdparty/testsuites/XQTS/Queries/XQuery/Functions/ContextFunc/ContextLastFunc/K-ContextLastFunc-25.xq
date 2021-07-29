(:*******************************************************:)
(: Test: K-ContextLastFunc-25                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: fn:last() inside a predicate combined with a offset. :)
(:*******************************************************:)
(1, 2, 3, 4, current-time(), 4, 5, 6)[last() - 1] treat as xs:integer eq 5