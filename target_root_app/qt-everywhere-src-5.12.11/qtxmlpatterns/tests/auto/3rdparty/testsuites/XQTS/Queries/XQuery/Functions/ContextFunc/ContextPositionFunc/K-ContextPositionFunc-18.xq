(:*******************************************************:)
(: Test: K-ContextPositionFunc-18                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: The return value of fn:position() is always greater or equal to 0('>='). :)
(:*******************************************************:)
deep-equal(
(1, 2, 3, remove((current-time(), 4), 1))
[position() >= 1],
(1, 2, 3, 4))