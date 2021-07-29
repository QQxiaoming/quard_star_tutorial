export function runTest(libraryUnderTest) {
    let state1 = state(libraryUnderTest);
    try { modifyFromOutside(libraryUnderTest); } catch (e) {}
    let state2 = state(libraryUnderTest);
    try { modifyFromInside(libraryUnderTest); } catch (e) {}
    let state3 = state(libraryUnderTest);
    return state1 + " " + state2 + " " + state3;
}

function stringify(value) {
    let s = "?";
    if (value !== undefined)
        s = value.toString();
    return s;
}

function state(libraryUnderTest) {
    return (stringify(libraryUnderTest.varValue) +
            stringify(libraryUnderTest.letValue) +
            stringify(libraryUnderTest.constValue));
}

function modifyFromOutside(libraryUnderTest) {
    ++libraryUnderTest.varValue;
    ++libraryUnderTest.letValue;
    ++libraryUnderTest.constValue;
}

function modifyFromInside(libraryUnderTest) {
    libraryUnderTest.incrementAll();
}
