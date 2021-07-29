(:*******************************************************:)
(: Test: K-ContextPositionFunc-8                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: fn:position() can never return 0('ne').      :)
(:*******************************************************:)
deep-equal(
(1, 2, 3, remove((current-time(), 4), 1))
[position() ne 0],
(1, 2, 3, 4))