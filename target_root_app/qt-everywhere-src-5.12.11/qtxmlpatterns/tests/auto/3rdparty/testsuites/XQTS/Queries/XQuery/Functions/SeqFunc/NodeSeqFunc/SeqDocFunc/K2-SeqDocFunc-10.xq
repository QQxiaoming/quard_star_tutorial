(:*******************************************************:)
(: Test: K2-SeqDocFunc-10                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:doc() with a static base-uri that is complete, but doesn't point to a file. :)
(:*******************************************************:)
declare base-uri "file:///directory/directory/example.com/www.example.com/thisFileDoesNotExist.xml";
doc("")