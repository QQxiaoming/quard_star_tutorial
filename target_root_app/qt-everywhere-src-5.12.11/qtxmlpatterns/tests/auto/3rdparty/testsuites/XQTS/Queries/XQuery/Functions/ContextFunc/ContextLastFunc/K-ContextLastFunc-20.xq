(:*******************************************************:)
(: Test: K-ContextLastFunc-20                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: The return value of fn:last() is always greater than 0('>'). :)
(:*******************************************************:)
deep-equal(
(1, 2, 3, remove((current-time(), 4), 1))
[last() > 0],
(1, 2, 3, 4))