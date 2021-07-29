(:*******************************************************:)
(: Test: K2-ConText-14                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that an empty text node between two atomic values are serialized correctly. :)
(:*******************************************************:)
<elem>{1}{text{""}}{2}</elem>