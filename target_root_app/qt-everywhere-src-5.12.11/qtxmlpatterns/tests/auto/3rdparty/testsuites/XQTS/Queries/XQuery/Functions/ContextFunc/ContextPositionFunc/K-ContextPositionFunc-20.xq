(:*******************************************************:)
(: Test: K-ContextPositionFunc-20                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: position() combined with a comparison operator inside a predicate. :)
(:*******************************************************:)
1 eq (0, 1, current-time(), 4)[position() = 2] treat as xs:integer