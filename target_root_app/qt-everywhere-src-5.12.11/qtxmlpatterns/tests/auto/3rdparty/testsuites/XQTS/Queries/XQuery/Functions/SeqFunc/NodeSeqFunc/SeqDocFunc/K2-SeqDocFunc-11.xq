(:*******************************************************:)
(: Test: K2-SeqDocFunc-11                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:doc() with a static base-uri that points to a non-existent directory. :)
(:*******************************************************:)
declare base-uri "file:///directory/directory/example.com/";
doc("")