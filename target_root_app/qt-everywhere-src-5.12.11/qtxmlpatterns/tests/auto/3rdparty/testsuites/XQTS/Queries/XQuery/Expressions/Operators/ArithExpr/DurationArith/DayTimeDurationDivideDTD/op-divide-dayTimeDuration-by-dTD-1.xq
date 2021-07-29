(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-1             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-DTD" operator :)
(:As per example 1 (for this function)of the F&O specs.  :)
(:*******************************************************:)

fn:round-half-to-even((xs:dayTimeDuration("P2DT53M11S") div xs:dayTimeDuration("P1DT10H")),15)

