"use strict";

Object.defineProperty(exports, "__esModule", {
    value: true
});
exports.UInt53toBufferLE = undefined;

var _slicedToArray = function () { function sliceIterator(arr, i) { var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"]) _i["return"](); } finally { if (_d) throw _e; } } return _arr; } return function (arr, i) { if (Array.isArray(arr)) { return arr; } else if (Symbol.iterator in Object(arr)) { return sliceIterator(arr, i); } else { throw new TypeError("Invalid attempt to destructure non-iterable instance"); } }; }();

var _assert = require("assert");

var _assert2 = _interopRequireDefault(_assert);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

/*
 * Originally based on code from github:KhaosT/HAP-NodeJS@0c8fd88 used
 * used per the terms of the Apache Software License v2.
 *
 * Original code copyright Khaos Tian <khaos.tian@gmail.com>
 *
 * Modifications copyright Zach Bean <zb@forty2.com>
 *  * Reformatted for ES6-style module
 *  * renamed *UInt64* to *UInt53* to be more clear about range
 *  * renamed uintHighLow to be more clear about what it does
 *  * Refactored to return a buffer rather write into a passed-in buffer
 */

function splitUInt53(number) {
    const MAX_UINT32 = 0x00000000FFFFFFFF;
    const MAX_INT53 = 0x001FFFFFFFFFFFFF;

    (0, _assert2.default)(number > -1 && number <= MAX_INT53, "number out of range");
    (0, _assert2.default)(Math.floor(number) === number, "number must be an integer");

    var high = 0;
    var signbit = number & 0xFFFFFFFF;
    var low = signbit < 0 ? (number & 0x7FFFFFFF) + 0x80000000 : signbit;

    if (number > MAX_UINT32) {
        high = (number - low) / (MAX_UINT32 + 1);
    }
    return [high, low];
}

function UInt53toBufferLE(number) {
    var _splitUInt = splitUInt53(number),
        _splitUInt2 = _slicedToArray(_splitUInt, 2);

    const high = _splitUInt2[0],
          low = _splitUInt2[1];


    const buf = Buffer.alloc(8);
    buf.writeUInt32LE(low, 0);
    buf.writeUInt32LE(high, 4);

    return buf;
}

exports.UInt53toBufferLE = UInt53toBufferLE;
//# sourceMappingURL=data:application/json;charset=utf-8;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbIi4uLy4uL3NyYy9saWIvbnVtYmVyLmpzIl0sIm5hbWVzIjpbInNwbGl0VUludDUzIiwibnVtYmVyIiwiTUFYX1VJTlQzMiIsIk1BWF9JTlQ1MyIsIk1hdGgiLCJmbG9vciIsImhpZ2giLCJzaWduYml0IiwibG93IiwiVUludDUzdG9CdWZmZXJMRSIsImJ1ZiIsIkJ1ZmZlciIsImFsbG9jIiwid3JpdGVVSW50MzJMRSJdLCJtYXBwaW5ncyI6Ijs7Ozs7Ozs7O0FBQUE7Ozs7OztBQUVBOzs7Ozs7Ozs7Ozs7O0FBYUEsU0FBU0EsV0FBVCxDQUFxQkMsTUFBckIsRUFBNkI7QUFDekIsVUFBTUMsYUFBYSxrQkFBbkI7QUFDQSxVQUFNQyxZQUFhLGtCQUFuQjs7QUFFQSwwQkFBT0YsU0FBUyxDQUFDLENBQVYsSUFBZUEsVUFBVUUsU0FBaEMsRUFBMkMscUJBQTNDO0FBQ0EsMEJBQU9DLEtBQUtDLEtBQUwsQ0FBV0osTUFBWCxNQUF1QkEsTUFBOUIsRUFBc0MsMkJBQXRDOztBQUVBLFFBQUlLLE9BQU8sQ0FBWDtBQUNBLFFBQUlDLFVBQVVOLFNBQVMsVUFBdkI7QUFDQSxRQUFJTyxNQUFNRCxVQUFVLENBQVYsR0FBYyxDQUFDTixTQUFTLFVBQVYsSUFBd0IsVUFBdEMsR0FBbURNLE9BQTdEOztBQUVBLFFBQUlOLFNBQVNDLFVBQWIsRUFBeUI7QUFDckJJLGVBQU8sQ0FBQ0wsU0FBU08sR0FBVixLQUFrQk4sYUFBYSxDQUEvQixDQUFQO0FBQ0g7QUFDRCxXQUFPLENBQUVJLElBQUYsRUFBUUUsR0FBUixDQUFQO0FBQ0g7O0FBRUQsU0FBU0MsZ0JBQVQsQ0FBMEJSLE1BQTFCLEVBQWtDO0FBQUEscUJBQ1JELFlBQVlDLE1BQVosQ0FEUTtBQUFBOztBQUFBLFVBQ3RCSyxJQURzQjtBQUFBLFVBQ2hCRSxHQURnQjs7O0FBRzlCLFVBQU1FLE1BQU1DLE9BQU9DLEtBQVAsQ0FBYSxDQUFiLENBQVo7QUFDQUYsUUFBSUcsYUFBSixDQUFrQkwsR0FBbEIsRUFBd0IsQ0FBeEI7QUFDQUUsUUFBSUcsYUFBSixDQUFrQlAsSUFBbEIsRUFBd0IsQ0FBeEI7O0FBRUEsV0FBT0ksR0FBUDtBQUNIOztRQUdHRCxnQixHQUFBQSxnQiIsImZpbGUiOiJudW1iZXIuanMiLCJzb3VyY2VzQ29udGVudCI6WyJpbXBvcnQgYXNzZXJ0IGZyb20gJ2Fzc2VydCc7XG5cbi8qXG4gKiBPcmlnaW5hbGx5IGJhc2VkIG9uIGNvZGUgZnJvbSBnaXRodWI6S2hhb3NUL0hBUC1Ob2RlSlNAMGM4ZmQ4OCB1c2VkXG4gKiB1c2VkIHBlciB0aGUgdGVybXMgb2YgdGhlIEFwYWNoZSBTb2Z0d2FyZSBMaWNlbnNlIHYyLlxuICpcbiAqIE9yaWdpbmFsIGNvZGUgY29weXJpZ2h0IEtoYW9zIFRpYW4gPGtoYW9zLnRpYW5AZ21haWwuY29tPlxuICpcbiAqIE1vZGlmaWNhdGlvbnMgY29weXJpZ2h0IFphY2ggQmVhbiA8emJAZm9ydHkyLmNvbT5cbiAqICAqIFJlZm9ybWF0dGVkIGZvciBFUzYtc3R5bGUgbW9kdWxlXG4gKiAgKiByZW5hbWVkICpVSW50NjQqIHRvICpVSW50NTMqIHRvIGJlIG1vcmUgY2xlYXIgYWJvdXQgcmFuZ2VcbiAqICAqIHJlbmFtZWQgdWludEhpZ2hMb3cgdG8gYmUgbW9yZSBjbGVhciBhYm91dCB3aGF0IGl0IGRvZXNcbiAqICAqIFJlZmFjdG9yZWQgdG8gcmV0dXJuIGEgYnVmZmVyIHJhdGhlciB3cml0ZSBpbnRvIGEgcGFzc2VkLWluIGJ1ZmZlclxuICovXG5cbmZ1bmN0aW9uIHNwbGl0VUludDUzKG51bWJlcikge1xuICAgIGNvbnN0IE1BWF9VSU5UMzIgPSAweDAwMDAwMDAwRkZGRkZGRkZcbiAgICBjb25zdCBNQVhfSU5UNTMgPSAgMHgwMDFGRkZGRkZGRkZGRkZGXG5cbiAgICBhc3NlcnQobnVtYmVyID4gLTEgJiYgbnVtYmVyIDw9IE1BWF9JTlQ1MywgXCJudW1iZXIgb3V0IG9mIHJhbmdlXCIpXG4gICAgYXNzZXJ0KE1hdGguZmxvb3IobnVtYmVyKSA9PT0gbnVtYmVyLCBcIm51bWJlciBtdXN0IGJlIGFuIGludGVnZXJcIilcblxuICAgIHZhciBoaWdoID0gMFxuICAgIHZhciBzaWduYml0ID0gbnVtYmVyICYgMHhGRkZGRkZGRlxuICAgIHZhciBsb3cgPSBzaWduYml0IDwgMCA/IChudW1iZXIgJiAweDdGRkZGRkZGKSArIDB4ODAwMDAwMDAgOiBzaWduYml0XG5cbiAgICBpZiAobnVtYmVyID4gTUFYX1VJTlQzMikge1xuICAgICAgICBoaWdoID0gKG51bWJlciAtIGxvdykgLyAoTUFYX1VJTlQzMiArIDEpXG4gICAgfVxuICAgIHJldHVybiBbIGhpZ2gsIGxvdyBdXG59XG5cbmZ1bmN0aW9uIFVJbnQ1M3RvQnVmZmVyTEUobnVtYmVyKSB7XG4gICAgY29uc3QgWyBoaWdoLCBsb3cgXSA9IHNwbGl0VUludDUzKG51bWJlcilcblxuICAgIGNvbnN0IGJ1ZiA9IEJ1ZmZlci5hbGxvYyg4KTtcbiAgICBidWYud3JpdGVVSW50MzJMRShsb3csICAwKTtcbiAgICBidWYud3JpdGVVSW50MzJMRShoaWdoLCA0KTtcblxuICAgIHJldHVybiBidWY7XG59XG5cbmV4cG9ydCB7XG4gICAgVUludDUzdG9CdWZmZXJMRVxufVxuIl19