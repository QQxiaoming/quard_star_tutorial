(:*******************************************************:)
(: Test: K-ContextLastFunc-12                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: fn:last() can never return 0('ne').          :)
(:*******************************************************:)
deep-equal(
(1, 2, 3, remove((current-time(), 4), 1))
[last() ne 0],
(1, 2, 3, 4))