(:*******************************************************:)
(: Test: K-ContextPositionFunc-22                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: position() combined with a comparison operator inside a predicate. :)
(:*******************************************************:)
1 eq (0, 1, current-time(), 4)[2 eq position()] treat as xs:integer