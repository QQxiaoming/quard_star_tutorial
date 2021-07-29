(:*******************************************************:)
(: Test: K-ContextLastFunc-13                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: fn:last() can never return 0('!=').          :)
(:*******************************************************:)
deep-equal(
(1, 2, 3, remove((current-time(), 4), 1))
[0 != last()],
(1, 2, 3, 4))