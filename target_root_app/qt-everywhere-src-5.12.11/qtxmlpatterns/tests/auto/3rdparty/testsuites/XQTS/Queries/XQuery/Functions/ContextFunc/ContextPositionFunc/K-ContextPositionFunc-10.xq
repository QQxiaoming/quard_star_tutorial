(:*******************************************************:)
(: Test: K-ContextPositionFunc-10                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: fn:position() can never return 0('ne'), #2.  :)
(:*******************************************************:)
deep-equal(
(1, 2, 3, remove((current-time(), 4), 1))
[0 ne position()],
(1, 2, 3, 4))