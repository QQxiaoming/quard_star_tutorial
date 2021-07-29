(:Test: hexBinary-3                                       :)
(:Description: Simple Binary hex opeartion as part of argument to fn:not function. :)

fn:not((xs:hexBinary("786174616d61616772") eq xs:hexBinary("786174616d61616772")))