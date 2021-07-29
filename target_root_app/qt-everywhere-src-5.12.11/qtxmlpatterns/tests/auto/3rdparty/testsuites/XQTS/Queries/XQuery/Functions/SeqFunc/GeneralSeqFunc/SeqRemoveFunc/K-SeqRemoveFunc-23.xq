(:*******************************************************:)
(: Test: K-SeqRemoveFunc-23                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: Apply a predicate to the result of fn:remove(). :)
(:*******************************************************:)
remove((1, 2, 3, current-time()), 9)[last() - 1]