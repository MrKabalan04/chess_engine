// components/MoveHistory.jsx
import './MoveHistory.css';

function MoveHistory({ moves }) {
  return (
    <div className="move-history">
      <h3>Move History</h3>
      <div className="move-list">
        <table>
          <thead>
            <tr>
              <th>#</th>
              <th>White</th>
              <th>Black</th>
            </tr>
          </thead>
          <tbody>
            {moves.map((move, idx) => (
              <tr key={idx}>
                <td>{idx + 1}</td>
                <td>{move.white || '...'}</td>
                <td>{move.black || '...'}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default MoveHistory;