(:*******************************************************:)
(: Test: K-SeqRemoveFunc-16                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: Combine remove(), with a predicate and the 'eq' operator. :)
(:*******************************************************:)
deep-equal((1, 2), (1, 2)[remove((true(), "a string"), 2)]) eq 0