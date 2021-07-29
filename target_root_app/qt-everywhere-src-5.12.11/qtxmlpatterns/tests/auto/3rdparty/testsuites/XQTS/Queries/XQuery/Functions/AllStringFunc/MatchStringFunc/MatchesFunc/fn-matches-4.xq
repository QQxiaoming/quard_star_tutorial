(: Name: fn-matches-4 :)
(: Description: Test that calling the function with flags set to the empty string is the same as ommiting the flags.:)

fn:concat(fn:matches("abracadabra", "^bra"),fn:matches("abracadabra", "^bra", ""))