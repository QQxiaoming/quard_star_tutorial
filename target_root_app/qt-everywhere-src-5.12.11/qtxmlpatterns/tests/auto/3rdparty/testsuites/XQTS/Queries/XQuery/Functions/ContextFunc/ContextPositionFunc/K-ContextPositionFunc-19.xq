(:*******************************************************:)
(: Test: K-ContextPositionFunc-19                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: The return value of fn:position() is always greater or equal to 0('ge'). :)
(:*******************************************************:)
deep-equal(
(1, 2, 3, remove((current-time(), 4), 1))
[position() ge 1],
(1, 2, 3, 4))