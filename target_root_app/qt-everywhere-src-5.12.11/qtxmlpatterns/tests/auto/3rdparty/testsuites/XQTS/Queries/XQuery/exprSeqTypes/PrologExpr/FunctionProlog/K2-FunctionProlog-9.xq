(:*******************************************************:)
(: Test: K2-FunctionProlog-9                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A recursive function multiplying a sequence. :)
(:*******************************************************:)
declare function local:multiplySequence($input as xs:integer*) as xs:integer
{
    if      (empty($input))
    then    1
    else    $input[1] * local:multiplySequence($input[position() != 1])
};
local:multiplySequence((1, 2, 3, 4, 5))